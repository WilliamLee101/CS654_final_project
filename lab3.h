//
// CS 454/654 - Lab 3 Defines
// PC/AVR Board
//
// Objective:
//   Abstract common definitions between server, client and troll
//
//   02/11/19  Modified. -Renato Mancuso (BU)
//

#ifndef _LAB_03_H
#define _LAB_03_H

#define MSG_START		0x0		// Start byte value
#define MSG_BYTES_START		1		// Start byte
#define MSG_BYTES_CRC		2		// CRC length
#define MSG_BYTES_MSG_LEN	1		// Message len (N)
#define MSG_BYTES_HEADER	MSG_BYTES_START+MSG_BYTES_CRC+MSG_BYTES_MSG_LEN
#define MSG_BYTES_MSG		10		// 10 byte packets
#define MSG_MAX_LEN		MSG_BYTES_HEADER+MSG_BYTES_MSG

#define MSG_ACK			1
#define MSG_NACK		0

#define END_OF_DATA_FLAG 0xFF
#define END_OF_DATA_PAD 0x00
#define MAX_DURATION 10

#define CONFIG_PACKET_SIZE 2


#endif



