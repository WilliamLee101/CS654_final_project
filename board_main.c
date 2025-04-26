/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define MSG_START		0x0		// Start byte value
#define MSG_BYTES_START		1		// Start byte
#define MSG_BYTES_CRC		2		// CRC length
#define MSG_BYTES_MSG_LEN	1		// Message len (N)
#define MSG_BYTES_HEADER	MSG_BYTES_START + MSG_BYTES_CRC + MSG_BYTES_MSG_LEN
#define MSG_BYTES_MSG		10		// Message
#define MSG_MAX_LEN		MSG_BYTES_HEADER + MSG_BYTES_MSG
#define MSG_ACK			1
#define MSG_NACK		0

#define END_OF_DATA_FLAG 0xFF
#define END_OF_DATA_PAD 0x00
#define CONFIG_PACKET_SIZE 2


volatile bool timeout_flag = false;
volatile bool led_timeout_flag = false;

void TMR2Callback(uint32_t status, uintptr_t context) {
    timeout_flag = true;
    LED10_Toggle();
    TMR2_Stop();
}

void TMR3Callback(uint32_t status, uintptr_t context) {
    led_timeout_flag = true;
    TMR3_Stop();
}

void enable_output_all_leds() {
    LED3_OutputEnable();
    LED4_OutputEnable();
    LED5_OutputEnable();
    LED6_OutputEnable();
    LED7_OutputEnable();
    LED8_OutputEnable();
    LED9_OutputEnable();
    LED10_OutputEnable();
}

void clear_all_leds() {
    LED3_Clear();
    LED4_Clear();
    LED5_Clear();
    LED6_Clear();
    LED7_Clear();
    LED8_Clear();
    LED9_Clear();
    LED10_Clear();
}

void delay_for(uint8_t x) {
    // use TMR3 to delay   
    uint8_t i;
    for (i = 0; i < x; i++) {
        TMR3_Start();
        while (!led_timeout_flag);
        led_timeout_flag = false;
    }
}

// CRC function gotten from the lab files
uint16_t crc_update(uint16_t crc, uint8_t data)     {
	int16_t i;
	crc ^= data;
	for (i = 0; i < 8; ++i)	{
	    if (crc & 1) crc = (crc >> 1) ^ 0xA001;
	    else crc = (crc >> 1);
	}
	return crc;
}

void turn_on_led_for_number(uint8_t number) {
    switch (number) {
        case 3: LED3_Set(); break;
        case 4: LED4_Set(); break;
        case 5: LED5_Set(); break;
        case 6: LED6_Set(); break;
        case 7: LED7_Set(); break;
        case 8: LED8_Set(); break;
        case 9: LED9_Set(); break;
        case 10: LED10_Set(); break;
        default: break;
    }
}

void byte_to_led(uint8_t byte) {
    for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++) {
        if (byte & (1 << bit_idx)) {
            turn_on_led_for_number(3 + bit_idx); // LEDs 3–10 for bits 0–7
        }
    }
}

bool is_end_of_data(uint8_t* packet, uint8_t n, uint8_t max_n) {
    if (n != max_n || packet[0] != END_OF_DATA_FLAG) return false;
    
    uint8_t i;
    for (i = 0; i < n - 1; i++) {
        if (packet[i + 1] != END_OF_DATA_PAD) return false;
    }
    
    return true;
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    enable_output_all_leds();
    clear_all_leds();
    
    // set up TMR 2 and TMR 3
    TMR2_CallbackRegister(TMR2Callback, (uintptr_t) NULL);
    TMR3_CallbackRegister(TMR3Callback, (uintptr_t) NULL);
    
    // wait for CONFIG packet
    uint8_t packet_size, duration;
    bool config_received = false; 
    uint8_t config_buf[CONFIG_PACKET_SIZE];
    
    uint8_t start_byte;
    uint8_t msg_len;
    uint16_t crc, crc_received;
    uint8_t i;
    uint8_t crc_high = 0, crc_low = 0;
    
    while(!config_received) {
        // wait to receive the start byte from the server + start timer once received
        while(1) if (UART5_Read(&start_byte, 1)) break;
        TMR2_Start();
                
        // receive 16 bit CRC high to low bytes
        while (!timeout_flag) {
            if (UART5_Read(&crc_high, 1)) break;
        }
        
        while (!timeout_flag) {
            if (UART5_Read(&crc_low, 1)) break;
        }
        
        // wait to receive the length of the message
        while (!timeout_flag) {
            if (UART5_Read(&msg_len, 1)) break;
        }
        
        if (msg_len > CONFIG_PACKET_SIZE) {
            TMR2_Stop();
            timeout_flag = false;
            UART5_WriteByte(MSG_NACK);
            continue;
        }
        
        // read in the rest of the message and stop the timer after
        uint8_t bytes_received = 0;
        while (!timeout_flag && bytes_received < msg_len) {
            if (UART5_Read(&config_buf[bytes_received], 1)) bytes_received++;
        }
        
        TMR2_Stop();
        
        // handle data corruption       
        if (start_byte != MSG_START || timeout_flag || bytes_received != msg_len) {
            timeout_flag = false;
            UART5_WriteByte(MSG_NACK);
            continue;
        }
        
        // process the received CRC
        crc_received = crc_high;
        crc_received <<= 8;
        crc_received |= crc_low;
        
        // calculate the CRC AFTER receiving all of the data
        crc = 0;
        for (i = 0; i < msg_len; i++) crc = crc_update(crc, config_buf[i]);
        
        // compare CRC and send ACK/NACK accordingly
        if (crc == crc_received) {
            packet_size = config_buf[0];
            duration = config_buf[1];
            
            timeout_flag = false;
            UART5_WriteByte(MSG_ACK); // Send ACK
            break;
        } else {            
            // handle data corruption
            timeout_flag = false;
            UART5_WriteByte(MSG_NACK); // Send NACK
        }
    }    
    
    uint8_t buf[packet_size];
    
    while ( true ) {            
        // wait to receive the start byte from the server + start timer once received
        while(1) if (UART5_Read(&start_byte, 1)) break;
        TMR2_Start();
                
        // receive 16 bit CRC high to low bytes
        while (!timeout_flag) {
            if (UART5_Read(&crc_high, 1)) break;
        }
        
        while (!timeout_flag) {
            if (UART5_Read(&crc_low, 1)) break;
        }
        
        // wait to receive the length of the message
        while (!timeout_flag) {
            if (UART5_Read(&msg_len, 1)) break;
        }
        
        if (msg_len > packet_size) {
            TMR2_Stop();
            timeout_flag = false;
            SYS_Tasks ( );
            UART5_WriteByte(MSG_NACK);
            continue;
        }
        
        // read in the rest of the message and stop the timer after
        uint8_t bytes_received = 0;
        
        while (!timeout_flag && bytes_received < msg_len) {
            if (UART5_Read(&buf[bytes_received], 1)) bytes_received++;
        }
        
        TMR2_Stop();
        
        // handle data corruption       
        if (start_byte != MSG_START || timeout_flag || bytes_received != msg_len) {
            timeout_flag = false;
            SYS_Tasks ( );
            UART5_WriteByte(MSG_NACK);
            continue;
        }
        
        // check if END_OF_DATA is received
        if (is_end_of_data(buf, msg_len, packet_size)) {
            clear_all_leds();
            timeout_flag = false;
            SYS_Tasks();
            UART5_WriteByte(MSG_ACK); // Send ACK
            break;
        }
        
        // process the received CRC
        crc_received = crc_high;
        crc_received <<= 8;
        crc_received |= crc_low;
        
        // calculate the SRS AFTER receiving all of the data
        crc = 0;
        for (i = 0; i < msg_len; i++) crc = crc_update(crc, buf[i]);
        
        // compare CRC and send ACK/NACK accordingly
        if (crc == crc_received) {
            // flash the bit representation of each byte received to LEDs
            uint8_t byte_idx;
            for (byte_idx = 0; byte_idx < msg_len; byte_idx++) {
                clear_all_leds();
                delay_for(duration);
                
                byte_to_led(buf[byte_idx]);
                delay_for(duration);
            }
            
            timeout_flag = false;
            SYS_Tasks ( );
            UART5_WriteByte(MSG_ACK); // Send ACK
        } else {            
            // handle data corruption
            timeout_flag = false;
            SYS_Tasks ( );
            UART5_WriteByte(MSG_NACK); // Send NACK
        }
    }

    /* Execution should not come here during normal operation */
    clear_all_leds();
    
    return ( EXIT_SUCCESS );
}


/*******************************************************************************
 End of File
*/

