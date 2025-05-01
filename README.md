# CS 654 Final Project

### Contributors: Eric Nohara-LeClair, Yin-Ching Lee, Jason Ha

---

## Table of Contents

- [Project Description](#project-description)
- [MPLAB Harmony](#mplab-harmony)
- [How to Compile and Run](#how-to-compile-and-run)
- [Demo Videos](#demo-videos)
- [Project Structure](#project-structure)
- [Potential Improvements](#potential-improvements)

## Project Description

- Remake of lab 3b with UART serial communication with the board as the client and the laptop running a server script
- Rather than sending ASCII messages, we send raw bytes from the server to the board
  - The server reads in a file's raw bytes byte by byte, generates packets of data, and sends these packets of bytes to the client board
  - The client board recieves these bytes, then processes the bytes into their bit representation, displaying the bits in each byte on the 8 LEDs
  - The cleint board then sends back a ACK/NACK byte following the same format as the lab
- The server and client work with the troll, using a start byte, CRC, message length, and timeout of 1 second to validate data
- The server allows configuration:
  - -f: allows you to specify the filename in the current directory to send to the board in bytes
  - -p: allows you to specify the packet size for the byte packets send to the board
  - -d: allows you to specify the duration that the LEDs flash when representing a single byte

## MPLAB Harmony

- Used MPLAB Harmony to generate firmware needed to interact with the 16/32 explorer board's LEDs, timers, and UART
- Used [Explorer 16/32 Schematics Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/BoardDesignFiles/Explorer_16_32_Schematics_R6_3.pdf) to find the pins that the PIM uses to interact with the board
- Used [PIM Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/Brochures/50002359A.pdf) to find how the pins on the 100 pin PIC32MZ interfaced with the 100 pin PIM
- Used [PIC32MZ Datasheet](https://www.mouser.com/datasheet/2/268/PIC32MZ_Embedded_Connectivity_with_Floating_Point_-3314567.pdf) to find which registers and pins need to be configured in MPLAB Harmony in order to set up timers, LEDs, and UART
- Informative videos used:
  - [Setting up LEDs](https://www.youtube.com/watch?v=dZkVQvZ6C1s&ab_channel=Nezbrun)
  - [Setting up UART](https://www.youtube.com/watch?v=k_dsdO2kWgg&t=87s&ab_channel=Nezbrun)

## How to Compile and Run

```
make all
./lab3_server /dev/tty<dev_num> -t 0.1 -p 10 -d 1 -f counting
```

## Demo Videos

- [Demo Video 1](./assets/demo1.MOV)
- [Demo Video 2](./assets/demo2.MOV)
