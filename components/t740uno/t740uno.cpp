#include "t740uno_component.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/application.h"

namespace esphome {
namespace t740uno {

static const char *const TAG = "t740uno_component.component";

void T740UNOComponent::setup() {
    ESP_LOGCONFIG(TAG, "Setting up T740UNO UART Component...");
    uart_->set_baud_rate(this->baud_rate_);
    uart_->set_parity(this->parity_);
    uart_->set_stop_bits(this->stop_bits_);
    uart_->set_data_bits(this->data_bits_);
    uart_->enable_rx_pin(this->rx_pin_);
    uart_->enable_tx_pin(this->tx_pin_);
    uart_->enable();
}

void T740UNOComponent::loop() {
    while (available()) {
        uint8_t byte;
        read_byte(&byte);
        rx_buffer_.push_back(byte);
    }

    while (!rx_buffer_.empty()) {
        uint8_t byte = rx_buffer_.front();
        rx_buffer_.pop_front();

        if (byte == 0x34) {
            ESP_LOGD(TAG, "Received 0x34 - Ring Active");
            this->ring_active_ = true;
            this->ring_sensor_last_active_time_ = TIME_COMPONENT->now().timestamp();
            if (this->ring_sensor_ != nullptr) {
                this->ring_sensor_->publish_state(true);
            }
        }
    }

    if (this->ring_active_ && this->ring_sensor_ != nullptr) {
        if ((TIME_COMPONENT->now().timestamp() - this->ring_sensor_last_active_time_) >= 90) {
            ESP_LOGD(TAG, "Ring timeout - Ring Inactive");
            this->ring_active_ = false;
            this->ring_sensor_->publish_state(false);
        }
    }
}


esphome::action::Action<> *T740UNOComponent::open_action() {
  struct T740UNOOpenAction : public esphome::action::Action<> {
    T740UNOOpenAction(T740UNOComponent *parent) : parent_(parent) {}

    void run(LocalVariables *vars) override {
        ESP_LOGD(TAG, "T740UNO Open Action: Sending 0x55");
        this->parent_->uart_->write(0x55);
    }

  protected:
    T740UNOComponent *parent_;
  };
  return new T740UNOOpenAction(this);
}


}  // namespace t740uno
}  // namespace esphome