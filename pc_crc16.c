//
// CS 454/654 - Lab 3 CRC
// PC (given)
//
// Change Log:
//   07/15/04  Created.  -Greg Dhuse/
//   02/11/19  Modified. -Renato Mancuso (BU)
//


/* (Extracted from a document edited by Chuck Forsberg of
 * Omen Technology in Portland, Oregon)
 *
 * This function calculates the CRC used by the XMODEM/CRC Protocol
 * The first argument is a pointer to the message block.
 * The second argument is the number of bytes in the message block.
 * The function returns an integer which contains the CRC.
 * The low order 16 bits are the coefficients of the CRC.
 */

#include <stdint.h>

uint16_t pc_crc16(uint8_t * data, int count)
{
	uint16_t crc = 0;
	int i;

	while (--count >= 0)
	{
		crc ^= *data++;

		for (i = 0; i < 8; ++i)
		{
			if (crc & 1) crc = (crc >> 1) ^ 0xA001;
			else crc = crc >> 1;
		}
		
	}

	return (crc & 0xFFFF);
}
