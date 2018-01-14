
PacketSerialLib
================

## Summary
A simple C and Python library for sending and receiving packets of data between an Arduino and a PC.

The transfer of data to and from the Arduino is non-blocking. There is no limitation on the contents 
of the data. The data is transferred in a robust fashion and recoverable fashion (using packets) which 
is a key requirement for many applications.

A lot of sketches and code uses simple readline() calls but this has a number of limitations both for Arduino and clients.
This library aims to address those limitations and make packet transfer a simple process for Arduino and host developers. 

## Features
* FSM based for robust packet receipt and rejection of malformed packets
* Non-blocking receipt of packets 
* No limitation on data sent in packets
* Arduino C and host Python library for bidirectional packet transfer
* Optional CRC generation and checking
* No dependency on any other libraries

## Description
The packet receipt functionality is implemented as a finite state machine (FSM) making its behaviour robust
and predictable as well as allowing it to handle automatic resyncing of the stream
in the event of dropped characters, corrupt packets or starting packet receipt out of sync with the sender.

To receive packets the application code simply calls the getData() function (which is non-blocking) in the event loop 
allowing the code to process other events while waiting for packets to be received.

As the payload data is wrapped in a packet so there is no limitation to the contents of the data 
e.g. "\n" can be in the payload as well as the start and end characters for the packet.

Only the data from well formed received packets are passed to the receiving client code.
Any data which cannot be parsed into a well formed packet is dropped silently.

For additional safety CRC generation and checking for the data is supported. 
The default CRC generation simply returns "0" and so CRC checking is effectively disabled.
Just overide the calcCRC() method to use whatever CRC mechanism you require.

## Getting Started
Burn to the Reflector sketch in the examples folder onto the Arduino. This simple receives packets from the 
host and then sends them back as well as periodically sending keep-alive packets.

Run the host test program passing in the com port for the Arduino i.e. python packetSerial.py COM4

You should see packets being sent between the host and the Arduino. 

        C:\dev\repos\libraries\PacketSerialLib>c:\Python27\python.exe packetSerial.py COM4
        Using port COM4
        TX Packet [Hello 0]
        RX Packet [Hello 0]
        TX Packet [Hello 1]
        RX Packet [Hello 1]
        TX Packet [Hello 2]
        RX Packet [Hello 2]
        TX Packet [Hello 3]
        RX Packet [Hello 3]
        TX Packet [Hello 4]
        RX Packet [Hello 4]
        RX Packet [Still alive!]
        TX Packet [Hello 5]
        RX Packet [Hello 5]


## Packet Structure
Packet structure is:

    STX                 Packet start character      '<'
    DATALEN             Payload size                e.g. "5"
    FSEP                Field seperator             ','
    DATALEN             Payload                     e.g. "Hello"
    FSEP                Field seperator             ','
    CRC                 CRC                         e.g. "0"

A typical packet would look like:

    <5,Hello,0>
    


    
    
   