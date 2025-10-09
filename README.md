2600 8 bit  even parity  1 stop bit hex 
Fist 2 bytes: Unkown, seems allways to be 0
Third byte: address
Fourth byte: command


- Call from emiter to phone: '00 00 13 37'  - original message S00 S00 M13 M37
- Reponse to call(recieved incoming call from phone to emiter): '00 00 13 01' - original message S00 S00 M13 M01
- Picking up the phone after call ( from phone to emitter): 00 00 13 10  00 00 13 10  00 00 13 10  00 00 00 11 00 00 00 11 00 00 00 11 - 3 times '00 00 13 10' , then clear bus 3 times '00 00 00 11'
- after call, open door : 00 00 13 90  00 00 13 90  00 00 13 90   - 3 times '00 00 13 90' - original message S00 S00 M13 S90
- when press open the door and no comms active( from phone to emitter):  00 00 13 44  00 00 13 44  00 00 13 44  - 3 times '00 00 13 44' - original message S00 S00 M13 M44
- press open door from CE-941:
             00 00 00 47 00 00 00 47 00 00 00 47 - 3 times '00 00 00 47' sent by commander  -  original message S00 S00 S00 M47
             00 00 00 91 00 00 00 91 00 00 00 91 - 3 times '00 00 00 91' ??? response by door module - original message S00 S00 S00 S91
             00 00 00 46 00 00 00 46 00 00 00 46 - 3 times '00 00 00 46' ??? no clue - Original Message S00 S00 S00 M46
