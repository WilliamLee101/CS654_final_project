/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: template file to implement basic  */
/*                macros and functions to interface */
/*                with the on-board LEDs.           */
/*                                                  */
/****************************************************/

#ifndef __LED_H
#define __LED_H

// Tri-state registers
#define LEDTRIS		TRISA
#define LED3_TRIS	TRISAbits.TRISA0
#define LED4_TRIS	TRISAbits.TRISA1
#define LED5_TRIS	TRISAbits.TRISA2
#define LED6_TRIS   TRISAbits.TRISA3
#define LED7_TRIS   TRISFbits.TRISF13
#define LED8_TRIS   TRISFbits.TRISF12
#define LED9_TRIS   TRISAbits.TRISA6
#define LED10_TRIS  TRISAbits.TRISA7

// Port registers using predefined structs
#define LED_A_PORT		PORTA
#define LED_F_PORT      PORTF
#define LED3_PORT	PORTAbits.RA0
#define LED4_PORT	PORTAbits.RA1
#define LED5_PORT	PORTAbits.RA2
#define LED6_PORT	PORTAbits.RA3
#define LED7_PORT	PORTFbits.RF13
#define LED8_PORT	PORTFbits.RF12
#define LED9_PORT	PORTAbits.RA6
#define LED10_PORT	PORTAbits.RA7

// LEDPORT Bitwise definitions
#define LED3	0
#define LED4	1
#define LED5	2
#define LED6    3
#define LED7    13
#define LED8    12
#define LED9    6
#define LED10    7


//LED init
#define led_initialize() \
    do {                 \
        CLEARLED(LED3_TRIS); \
        CLEARLED(LED4_TRIS); \
        CLEARLED(LED5_TRIS); \
        CLEARLED(LED6_TRIS); \
        CLEARLED(LED7_TRIS); \
        CLEARLED(LED8_TRIS); \
        CLEARLED(LED9_TRIS); \
        CLEARLED(LED10_TRIS); \
        LED_A_PORT &= ~(BV(LED3) | BV(LED4)| BV(LED5)| BV(LED6) | BV(LED9) | BV(LED10)); \
        LED_F_PORT &= ~(BV(LED7) | BV(LED8)); \
    } while (0)

#endif
