/*******************************************************************************
  UART5 PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_uart5.c

  Summary:
    UART5 PLIB Implementation File

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#include "device.h"
#include "plib_uart5.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: UART5 Implementation
// *****************************************************************************
// *****************************************************************************


static void UART5_ErrorClear( void )
{
    UART_ERROR errors = UART_ERROR_NONE;
    uint8_t dummyData = 0u;

    errors = (UART_ERROR)(U5STA & (_U5STA_OERR_MASK | _U5STA_FERR_MASK | _U5STA_PERR_MASK));

    if(errors != UART_ERROR_NONE)
    {
        /* If it's a overrun error then clear it to flush FIFO */
        if((U5STA & _U5STA_OERR_MASK) != 0U)
        {
            U5STACLR = _U5STA_OERR_MASK;
        }

        /* Read existing error bytes from FIFO to clear parity and framing error flags */
        while((U5STA & _U5STA_URXDA_MASK) != 0U)
        {
            dummyData = (uint8_t)U5RXREG;
        }

    }

    // Ignore the warning
    (void)dummyData;
}

void UART5_Initialize( void )
{
    /* Set up UxMODE bits */
    /* STSEL  = 0 */
    /* PDSEL = 0 */
    /* UEN = 0 */

    U5MODE = 0x8;

    /* Enable UART5 Receiver and Transmitter */
    U5STASET = (_U5STA_UTXEN_MASK | _U5STA_URXEN_MASK | _U5STA_UTXISEL1_MASK );

    /* BAUD Rate register Setup */
    U5BRG = 103;

    /* Turn ON UART5 */
    U5MODESET = _U5MODE_ON_MASK;
}

bool UART5_SerialSetup( UART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    bool status = false;
    uint32_t baud;
    uint32_t status_ctrl;
    uint32_t uxbrg = 0;

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if ((baud == 0U) || ((setup->dataWidth == UART_DATA_9_BIT) && (setup->parity != UART_PARITY_NONE)))
        {
            return status;
        }

        if(srcClkFreq == 0U)
        {
            srcClkFreq = UART5_FrequencyGet();
        }

        /* Calculate BRG value */
        uxbrg = (((srcClkFreq >> 2) + (baud >> 1)) / baud);
        /* Check if the baud value can be set with low baud settings */
        if (uxbrg < 1U)
        {
            return status;
        }

        uxbrg -= 1U;

        if (uxbrg > UINT16_MAX)
        {
            return status;
        }

        /* Turn OFF UART5. Save UTXEN, URXEN and UTXBRK bits as these are cleared upon disabling UART */

        status_ctrl = U5STA & (_U5STA_UTXEN_MASK | _U5STA_URXEN_MASK | _U5STA_UTXBRK_MASK);

        U5MODECLR = _U5MODE_ON_MASK;

        if(setup->dataWidth == UART_DATA_9_BIT)
        {
            /* Configure UART5 mode */
            U5MODE = (U5MODE & (~_U5MODE_PDSEL_MASK)) | setup->dataWidth;
        }
        else
        {
            /* Configure UART5 mode */
            U5MODE = (U5MODE & (~_U5MODE_PDSEL_MASK)) | setup->parity;
        }

        /* Configure UART5 mode */
        U5MODE = (U5MODE & (~_U5MODE_STSEL_MASK)) | setup->stopBits;

        /* Configure UART5 Baud Rate */
        U5BRG = uxbrg;

        U5MODESET = _U5MODE_ON_MASK;

        /* Restore UTXEN, URXEN and UTXBRK bits. */
        U5STASET = status_ctrl;

        status = true;
    }

    return status;
}

bool UART5_AutoBaudQuery( void )
{
    bool autobaudqcheck = false;
    if((U5MODE & _U5MODE_ABAUD_MASK) != 0U)
    {

       autobaudqcheck = true;
    }
    return autobaudqcheck;
}

void UART5_AutoBaudSet( bool enable )
{
    if( enable == true )
    {
        U5MODESET = _U5MODE_ABAUD_MASK;
    }

    /* Turning off ABAUD if it was on can lead to unpredictable behavior, so that
       direction of control is not allowed in this function.                      */
}

bool UART5_Read(void* buffer, const size_t size )
{
    bool status = false;
    uint32_t errorStatus = 0;
    size_t processedSize = 0;

    if(buffer != NULL)
    {

        /* Clear error flags and flush out error data that may have been received when no active request was pending */
        UART5_ErrorClear();

        while( size > processedSize )
        {
            while((U5STA & _U5STA_URXDA_MASK) == 0U)
            {
                /* Wait for receiver to be ready */
            }

            /* Error status */
            errorStatus = (U5STA & (_U5STA_OERR_MASK | _U5STA_FERR_MASK | _U5STA_PERR_MASK));

            if(errorStatus != 0U)
            {
                break;
            }
            if (( U5MODE & (_U5MODE_PDSEL0_MASK | _U5MODE_PDSEL1_MASK)) == (_U5MODE_PDSEL0_MASK | _U5MODE_PDSEL1_MASK))
            {
                /* 9-bit mode */
                ((uint16_t*)(buffer))[processedSize] = (uint16_t)(U5RXREG );
            }
            else
            {
                /* 8-bit mode */
                ((uint8_t*)(buffer))[processedSize] = (uint8_t)(U5RXREG);
            }

            processedSize++;
        }

        if(size == processedSize)
        {
            status = true;
        }
    }

    return status;
}

bool UART5_Write( void* buffer, const size_t size )
{
    bool status = false;
    size_t processedSize = 0;

    if(buffer != NULL)
    {
        while( size > processedSize )
        {
            /* Wait while TX buffer is full */
            while ((U5STA & _U5STA_UTXBF_MASK) != 0U)
            {
                /* Wait for transmitter to be ready */
            }

            if (( U5MODE & (_U5MODE_PDSEL0_MASK | _U5MODE_PDSEL1_MASK)) == (_U5MODE_PDSEL0_MASK | _U5MODE_PDSEL1_MASK))
            {
                /* 9-bit mode */
                U5TXREG = ((uint16_t*)(buffer))[processedSize];
            }
            else
            {
                /* 8-bit mode */
                U5TXREG = ((uint8_t*)(buffer))[processedSize];
            }

            processedSize++;
        }

        status = true;
    }

    return status;
}

UART_ERROR UART5_ErrorGet( void )
{
    UART_ERROR errors = UART_ERROR_NONE;

    errors = (U5STA & (_U5STA_OERR_MASK | _U5STA_FERR_MASK | _U5STA_PERR_MASK));

    if(errors != UART_ERROR_NONE)
    {
        UART5_ErrorClear();
    }

    /* All errors are cleared, but send the previous error state */
    return errors;
}

void UART5_WriteByte(int data)
{
    while (((U5STA & _U5STA_UTXBF_MASK) != 0U))
    {
        /* Do Nothing */
    }

    U5TXREG = (uint32_t)data;
}

bool UART5_TransmitterIsReady( void )
{
    bool status = false;

    if((U5STA & _U5STA_UTXBF_MASK) == 0U)
    {
        status = true;
    }

    return status;
}

int UART5_ReadByte( void )
{
    return (int)(U5RXREG);
}

bool UART5_ReceiverIsReady( void )
{
    bool status = false;

    if(_U5STA_URXDA_MASK == (U5STA & _U5STA_URXDA_MASK))
    {
        status = true;
    }

    return status;
}

bool UART5_TransmitComplete( void )
{
    bool transmitComplete = false;

    if((U5STA & _U5STA_TRMT_MASK) != 0U)
    {
        transmitComplete = true;
    }

    return transmitComplete;
}