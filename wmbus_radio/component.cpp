#include "component.h"

#include "freertos/queue.h"
#include "freertos/task.h"

#define ASSERT(expr, expected, before_exit)                                    \
  {                                                                            \
    auto result = (expr);                                                      \
    if (!!result != expected) {                                                \
      ESP_LOGE(TAG, "Assertion failed: %s -> %d", #expr, result);              \
      before_exit;                                                             \
      return;                                                                  \
    }                                                                          \
  }

#define ASSERT_SETUP(expr) ASSERT(expr, 1, this->mark_failed())

namespace esphome {
namespace wmbus_radio {
static const char *TAG = "wmbus";

void Radio::setup() {
  ASSERT_SETUP(this->packet_queue_ = xQueueCreate(3, sizeof(Packet *)));

  ASSERT_SETUP(xTaskCreate((TaskFunction_t)this->receiver_task, "radio_recv",
                           3 * 1024, this, 2,
                           &(this->receiver_task_handle_)));

  ESP_LOGI(TAG, "Receiver task created [%p] - using GDO0 polling mode", this->receiver_task_handle_);

  // Note: We use polling for sync detection, but keep interrupt attached as backup
  this->radio->attach_data_interrupt(Radio::wakeup_receiver_task_from_isr,
                                     &(this->receiver_task_handle_));
}

void Radio::loop() {
  Packet *p;
  if (xQueueReceive(this->packet_queue_, &p, 0) != pdPASS)
    return;

  // ESP_LOGI(TAG, "Have RAW data from radio (%zu bytes)",
  //          p->calculate_payload_size());

  auto frame = p->convert_to_frame();

  if (!frame)
    return;

  ESP_LOGI(TAG, "Have data (%zu bytes) [RSSI: %ddBm, mode: %s %s]",
           frame->data().size(), frame->rssi(), toString(frame->link_mode()),
           frame->format().c_str());

  uint8_t packet_handled = 0;
  for (auto &handler : this->handlers_)
    handler(&frame.value());

  if (frame->handlers_count())
    ESP_LOGI(TAG, "Telegram handled by %d handlers", frame->handlers_count());
  else {
    ESP_LOGW(TAG, "Telegram not handled by any handler");
    Telegram t;
    if (t.parseHeader(frame->data()) && t.addresses.empty()) {
      ESP_LOGW(TAG, "Check if telegram can be parsed on:");
    } else {
      ESP_LOGW(TAG, "Check if telegram with address %s can be parsed on:",
               t.addresses.back().id.c_str());
    }
    ESP_LOGW(TAG,
             (std::string{"https://wmbusmeters.org/analyze/"} + frame->as_hex())
                 .c_str());
  }
}

void Radio::wakeup_receiver_task_from_isr(TaskHandle_t *arg) {
  BaseType_t xHigherPriorityTaskWoken;
  vTaskNotifyGiveFromISR(*arg, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Radio::receive_frame() {
  // Polling-based approach like ESPHome's CC1101 component
  // Poll GDO0 directly instead of relying on interrupts
  
  // Brief delay to allow radio to process
  vTaskDelay(pdMS_TO_TICKS(1));
  
  // Check if sync word was detected (GDO0 goes HIGH with IOCFG0=0x06)
  if (!this->radio->is_sync_detected()) {
    // No sync detected, wait a bit and return
    vTaskDelay(pdMS_TO_TICKS(5));
    return;
  }
  
  ESP_LOGD(TAG, "Sync word detected!");
  
  // Sync detected - now we need to read the packet
  // Wait for packet to arrive in FIFO
  uint32_t start_time = millis();
  const uint32_t PACKET_TIMEOUT_MS = 500;  // Max time to wait for packet data
  
  // Give some time for initial data to arrive
  vTaskDelay(pdMS_TO_TICKS(10));
  
  // Create packet to receive data
  auto packet = std::make_unique<Packet>();
  
  // First, try to read the preamble (first 3 bytes needed to determine frame type)
  size_t preamble_cap = packet->rx_capacity();
  ESP_LOGV(TAG, "Reading preamble (%zu bytes)", preamble_cap);
  
  if (!this->radio->read_in_task(packet->rx_data_ptr(), preamble_cap)) {
    ESP_LOGD(TAG, "Failed to read preamble");
    this->radio->clear_rx();
    return;
  }
  packet->rx_advance(preamble_cap);
  
  ESP_LOGV(TAG, "Got preamble: %02X %02X %02X", 
           packet->rx_data_ptr()[-3], packet->rx_data_ptr()[-2], packet->rx_data_ptr()[-1]);
  
  // Calculate total expected packet size
  if (!packet->calculate_payload_size()) {
    ESP_LOGD(TAG, "Cannot calculate payload size");
    this->radio->clear_rx();
    return;
  }
  
  // Read the remaining payload
  size_t payload_cap = packet->rx_capacity();
  ESP_LOGV(TAG, "Reading payload (%zu bytes)", payload_cap);
  
  if (payload_cap > 0) {
    if (!this->radio->read_in_task(packet->rx_data_ptr(), payload_cap)) {
      ESP_LOGW(TAG, "Failed to read full payload (wanted %zu bytes)", payload_cap);
      this->radio->clear_rx();
      return;
    }
    packet->rx_advance(payload_cap);
  }
  
  ESP_LOGI(TAG, "Received complete packet (%zu bytes)", 
           preamble_cap + payload_cap);
  
  // Success! Set RSSI and queue the packet
  packet->set_rssi(this->radio->get_rssi());
  auto packet_ptr = packet.get();
  
  if (xQueueSend(this->packet_queue_, &packet_ptr, 0) == pdTRUE) {
    ESP_LOGV(TAG, "Queue items: %zu", uxQueueMessagesWaiting(this->packet_queue_));
    packet.release();
  } else {
    ESP_LOGW(TAG, "Queue send failed");
  }
  
  // Prepare for next packet
  this->radio->restart_rx();
}

void Radio::receiver_task(Radio *arg) {
  int counter = 0;
  while (true)
    arg->receive_frame();
}

void Radio::add_frame_handler(std::function<void(Frame *)> &&callback) {
  this->handlers_.push_back(std::move(callback));
}

} // namespace wmbus_radio
} // namespace esphome
