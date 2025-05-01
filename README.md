# CS 654 Final Project

### Contributors: Eric Nohara-LeClair, Yin-Ching Lee, Jason Ha

---

## Table of Contents

- [Project Description](#project-description)
- [Procedure](#procedure)
- [MPLAB Harmony](#mplab-harmony)
- [Demo Videos](#demo-videos)
- [How to Compile and Run](#how-to-compile-and-run)
- [Known Issues](#known-issues)
- [Project Structure](#project-structure)

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

## Procedure

- Read over datasheets for the given board, PIM, and microchip
- Configure needed pins for TMR2, TMR3, UART5, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LED10 in MPLAB Harmony
- Generate the firmware peripheral files
- Edit board code from lab 3 to use the UART and timer functions defined in the generated firmware
- Edit byte receiving logic to allow for a config packet and to recieve packets of raw bytes
- Edit the lab3_server.c file from lab 3 to break up files of bytes into packets and send them to the board
- Test

## MPLAB Harmony

- Used MPLAB Harmony to generate firmware needed to interact with the 16/32 explorer board's LEDs, timers, and UART
- Used [Explorer 16/32 Schematics Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/BoardDesignFiles/Explorer_16_32_Schematics_R6_3.pdf) to find the pins that the PIM uses to interact with the board
- Used [PIM Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/Brochures/50002359A.pdf) to find how the pins on the 100 pin PIC32MZ interfaced with the 100 pin PIM
- Used [PIC32MZ Datasheet](https://www.mouser.com/datasheet/2/268/PIC32MZ_Embedded_Connectivity_with_Floating_Point_-3314567.pdf) to find which registers and pins need to be configured in MPLAB Harmony in order to set up timers, LEDs, and UART
- Informative videos used:
  - [Setting up LEDs](https://www.youtube.com/watch?v=dZkVQvZ6C1s&ab_channel=Nezbrun)
  - [Setting up UART](https://www.youtube.com/watch?v=k_dsdO2kWgg&t=87s&ab_channel=Nezbrun)

## Demo Videos

- [Demo Video 1](./assets/demo1.MOV)
- [Demo Video 2](./assets/demo2.MOV)

## How to Compile and Run

```
make all
./lab3_server /dev/tty<dev_num> -t 0.1 -p 10 -d 1 -f counting
```

## Known Issues

- After the server finishes sending the contents of a file, it sends an END_OF_DATA packet to the client. The client waits for this packet and exits execution upon receiving. It is possible for this packet to exist within a file of bytes being sent. In this case, the client will react to the packet and exit before receiving the rest of the data
- Must restart board program after the server completes before running again

## Project Structure

```
.
├── assets
│   ├── demo1.MOV
│   └── demo2.MOV
├── final_project.X
│   ├── build
│   │   └── default
│   │       ├── debug
│   │       │   └── _ext
│   │       │       ├── 1171490990
│   │       │       │   ├── exceptions.o
│   │       │       │   ├── exceptions.o.d
│   │       │       │   ├── initialization.o
│   │       │       │   ├── initialization.o.d
│   │       │       │   ├── interrupts.o
│   │       │       │   └── interrupts.o.d
│   │       │       ├── 1360937237
│   │       │       │   ├── main.o
│   │       │       │   └── main.o.d
│   │       │       ├── 163028504
│   │       │       │   ├── xc32_monitor.o
│   │       │       │   └── xc32_monitor.o.d
│   │       │       ├── 1865200349
│   │       │       │   ├── plib_evic.o
│   │       │       │   └── plib_evic.o.d
│   │       │       ├── 1865254177
│   │       │       │   ├── plib_gpio.o
│   │       │       │   └── plib_gpio.o.d
│   │       │       ├── 1865657120
│   │       │       │   ├── plib_uart5.o
│   │       │       │   └── plib_uart5.o.d
│   │       │       ├── 60165520
│   │       │       │   ├── plib_clk.o
│   │       │       │   └── plib_clk.o.d
│   │       │       └── 60181895
│   │       │           ├── plib_tmr2.o
│   │       │           ├── plib_tmr2.o.d
│   │       │           ├── plib_tmr3.o
│   │       │           └── plib_tmr3.o.d
│   │       └── production
│   │           └── _ext
│   │               ├── 1171490990
│   │               │   ├── exceptions.o
│   │               │   ├── exceptions.o.d
│   │               │   ├── initialization.o
│   │               │   ├── initialization.o.d
│   │               │   ├── interrupts.o
│   │               │   └── interrupts.o.d
│   │               ├── 1360937237
│   │               │   ├── main.o
│   │               │   └── main.o.d
│   │               ├── 163028504
│   │               │   ├── xc32_monitor.o
│   │               │   └── xc32_monitor.o.d
│   │               ├── 1865200349
│   │               │   ├── plib_evic.o
│   │               │   └── plib_evic.o.d
│   │               ├── 1865254177
│   │               │   ├── plib_gpio.o
│   │               │   └── plib_gpio.o.d
│   │               ├── 1865657120
│   │               │   ├── plib_uart2.o
│   │               │   ├── plib_uart2.o.d
│   │               │   ├── plib_uart5.o
│   │               │   └── plib_uart5.o.d
│   │               ├── 60165520
│   │               │   ├── plib_clk.o
│   │               │   └── plib_clk.o.d
│   │               └── 60181895
│   │                   ├── plib_tmr2.o
│   │                   ├── plib_tmr2.o.d
│   │                   ├── plib_tmr3.o
│   │                   └── plib_tmr3.o.d
│   ├── config
│   │   └── default
│   │       ├── default.mhc
│   │       ├── peripheral
│   │       │   ├── clk
│   │       │   │   ├── plib_clk.c
│   │       │   │   └── plib_clk.h
│   │       │   ├── evic
│   │       │   │   ├── plib_evic.c
│   │       │   │   └── plib_evic.h
│   │       │   ├── gpio
│   │       │   │   ├── plib_gpio.c
│   │       │   │   └── plib_gpio.h
│   │       │   ├── tmr
│   │       │   │   ├── plib_tmr2.c
│   │       │   │   ├── plib_tmr2.h
│   │       │   │   ├── plib_tmr3.c
│   │       │   │   ├── plib_tmr3.h
│   │       │   │   └── plib_tmr_common.h
│   │       │   └── uart
│   │       │       ├── plib_uart5.c
│   │       │       ├── plib_uart5.h
│   │       │       └── plib_uart_common.h
│   │       ├── stdio
│   │       │   └── xc32_monitor.c
│   │       ├── definitions.h
│   │       ├── device.h
│   │       ├── exceptions.c
│   │       ├── harmony-manifest-success.yml
│   │       ├── initialization.c
│   │       ├── interrupts.c
│   │       ├── interrupts.h
│   │       ├── p32MZ2048EFH100.ld
│   │       └── toolchain_specifics.h
│   ├── debug
│   │   └── default
│   ├── dist
│   │   └── default
│   │       ├── debug
│   │       │   ├── final_project.X.debug.elf
│   │       │   ├── final_project.X.debug.map
│   │       │   └── memoryfile.xml
│   │       └── production
│   │           ├── final_project.X.production.elf
│   │           ├── final_project.X.production.hex
│   │           ├── final_project.X.production.map
│   │           └── memoryfile.xml
│   ├── nbproject
│   │   ├── private
│   │   │   ├── configurations.xml
│   │   │   └── private.xml
│   │   ├── Makefile-default.mk
│   │   ├── Makefile-genesis.properties
│   │   ├── Makefile-impl.mk
│   │   ├── Makefile-local-default.mk
│   │   ├── Makefile-variables.mk
│   │   ├── configurations.xml
│   │   └── project.xml
│   ├── Makefile
│   ├── final_project.mc3
│   ├── main.c
│   ├── mcc-manifest-autosave.yml
│   └── mcc-manifest-generated-success.yml
├── utils
│   └── convert_to_8bit.py
├── Makefile
├── Makefile.pc
├── README.md
├── generate_sample_file.c
├── lab3.h
├── lab3_server.c
├── lab3_troll.c
├── lab3_troll.h
├── output
├── pc_crc16.c
├── pc_crc16.h
└── serialdebug.c
```
