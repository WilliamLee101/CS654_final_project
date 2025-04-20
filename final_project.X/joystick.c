#include "joystick.h"
#include <p33Fxxxx.h>
#include <xc.h>
#include <libpic30.h>

/**
 * @brief Initialize ADC2 for reading the joystick.
 *
 * Configures ADC2 for 10-bit, integer, automatic conversion.
 * Assumes that the hardware pins for AN4 and AN5 (RB4 and RB5) are used
 * for the joystick?s X and Y axes.
 */
void joystick_adc_init(void) {
    // Disable ADC2 for configuration
    AD2CON1bits.ADON = 0;
    
    // Set ADC2 to 10-bit mode with integer output
    AD2CON1bits.AD12B = 0;  // 10-bit mode
    AD2CON1bits.FORM = 0;   // Integer output
    AD2CON1bits.SSRC = 0x7; // Auto conversion trigger
    
    // No scanning (only one channel at a time)
    AD2CON2 = 0;
    
    // Configure sampling time and conversion clock:
    AD2CON3bits.ADRC = 0;   // Use internal clock
    AD2CON3bits.SAMC = 0x1F; // Sample time (31 Tad)
    AD2CON3bits.ADCS = 0x2;  // Tad = 3 Tcy
    
    // (Make sure the corresponding AN pins (AN4, AN5) are set as analog inputs
    // via the appropriate PCFG registers in your configuration code.)
    
    // Enable ADC2
    AD2CON1bits.ADON = 1;
}

/**
 * @brief Read the joystick?s X-axis value.
 *
 * Selects ADC2 channel 4 (AN4) and returns the ADC conversion result.
 *
 * @return uint16_t The ADC reading from the joystick X-axis.
 */
uint16_t joystick_get_x(void) {
    // Select channel AN4 for ADC2
    AD2CHS0bits.CH0SA = 4;  
    AD2CON1bits.SAMP = 1;   // Start sampling
    while (!AD2CON1bits.DONE); // Wait for conversion
    AD2CON1bits.DONE = 0;   // Clear the DONE flag
    return ADC2BUF0;        // Return the result
}

/**
 * @brief Read the joystick?s Y-axis value.
 *
 * Selects ADC2 channel 5 (AN5) and returns the ADC conversion result.
 *
 * @return uint16_t The ADC reading from the joystick Y-axis.
 */
uint16_t joystick_get_y(void) {
    // Select channel AN5 for ADC2
    AD2CHS0bits.CH0SA = 5;
    AD2CON1bits.SAMP = 1;   // Start sampling
    while (!AD2CON1bits.DONE); // Wait for conversion
    AD2CON1bits.DONE = 0;   // Clear the DONE flag
    return ADC2BUF0;        // Return the result
}