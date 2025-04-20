/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: template file to implement basic  */
/*                macros and functions to interface */
/*                with the analog joystick.         */
/*                                                  */
/****************************************************/

#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include <p33Fxxxx.h>
#include "types.h"

/* Please NOTE: */
/* BTN1 (BUTTON 1) is the "fire" button on the joystick */
/* BTN2 (BUTTON 2) is the "thumb" button on the joystick */

//check if BTN1 is pressed (active-low logic)
//PORTEbits.RE8 represents the state of BTN1
//pressed = 0 (low voltage; returns false)
//released = 1 (high voltage; returns true)
//reads the state of the hardware pin RE8 (on port E) where button 1 is connected
#define BTN1_PRESSED() \
    (!PORTEbits.RE8)
   
//check if BTN1 released
#define BTN1_RELEASED() \
    (PORTEbits.RE8)

//check if BTN2 is pressed (active-low logic)
//PORTDbits.RD10 represents the state of BTN2
//pressed = 0 (low voltage; returns false)
//released = 1 (high voltage; returns true)
//button 2 connected to RD10 on port D
#define BTN2_PRESSED() \
    (!PORTDbits.RD10)

//same thing as BTN1_RELEASED() but for BTN2
#define BTN2_RELEASED() \
    (PORTDbits.RD10)

//macro to set the 8 pin on port E to input for getting input from the joystick button
#define CONFIGURE_BTN1() do { \
    AD1PCFGHbits.PCFG20 = 1;  \
    TRISEbits.TRISE8 = 1;      \
} while(0)

//macro to set the 10 pin on port D to input for getting input from the joystick button
#define CONFIGURE_BTN2() do { \
    TRISDbits.TRISD10 = 1;  \
} while(0)

/* === Function prototypes implemented in joystick. go here === */


#endif /* __JOYSTICK_H */