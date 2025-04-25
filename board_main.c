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


volatile bool timeout_flag = false;

void TMR2Callback(uint32_t status, uintptr_t context) {
    timeout_flag = true;
    TMR2_Stop();
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
    
    // set up TMR 2
    TMR2_CallbackRegister(TMR2Callback, (uintptr_t) NULL);
    
    uint16_t crc, crc_received;

    uint8_t start_byte;
    uint8_t crc_high = 0, crc_low = 0;
    uint8_t msg_len;
    uint8_t buf[MSG_MAX_LEN];
    
    uint16_t failed_sends = 0;
    uint8_t i;
    
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        
        // wait to receive the start byte from the server + start timer once received
        while(!UART5_Read(&start_byte, 1));
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
        
        // read in the rest of the message and stop the timer after
        uint8_t bytes_received = 0;
        
        for (i = 0; i < msg_len; i++) {
            // wait for the next byte of data
            while (!timeout_flag) {
                if (UART5_Read(&buf[i], 1)) {
                    bytes_received++;
                    break;
                }
            }
        }
        
        TMR2_Stop();
        
        // handle data corruption       
        if (start_byte != MSG_START || timeout_flag || bytes_received != msg_len) {
            UART5_WriteByte(MSG_NACK);
            timeout_flag = false;
            failed_sends++;
            continue;
        }
        
        LED3_Toggle();
        
        // process the received CRC
        crc_received = crc_high;
        crc_received <<= 8;
        crc_received |= crc_low;
//        
//        // calculate the SRS AFTER receiving all of the data
        crc = 0;
        for (i = 0; i < msg_len; i++) crc = crc_update(crc, buf[i]);
        
        UART5_WriteByte(MSG_ACK); // Send ACK
//        
        // compare CRC and send ACK/NACK accordingly
//        if (crc == crc_received) {
//            UART5_WriteByte(MSG_ACK); // Send ACK
//            
//            // process the bytes in the buffer into a string
////            char message[msg_len + 1]; // +1 to null terminate the string
////            for (i = 0; i < msg_len; i++) message[i] = (char)buf[i];
////            message[msg_len] = '\0';
//            
//            // do some operation on the string
////            uint8_t num = atoi(message);         
////            if (num >= 3 && num <= 10) turn_on_led_for_number(num);
//            
//            failed_sends = 0; // reset counter for next message
//        } else {
//            // handle data corruption
//            UART5_WriteByte(MSG_NACK); // Send NACK
//            failed_sends++;
//            timeout_flag = false;
//        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

