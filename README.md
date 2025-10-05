2600 8 bit  even parity  1 stop bit hex 

- Call from emiter to phone: '00 00 13 37'  - address 13 command 37
- Reponse to call(recieved incoming call from phone to emiter): '00 00 13 01' 
- Picking up the phone after call ( from phone to emitter): 00 00 13 10  00 00 13 10  00 00 13 10  00 00 00 11 00 00 00 11 00 00 00 11 - 3 times '00 00 13 10' , then clear bus 3 times '00 00 00 11'
- Open door after picking up ( from phone to emitter):  00 00 13 44  00 00 13 44  00 00 13 44  - 3 times '00 00 13 44' ????
