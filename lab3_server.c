/****************************************************/
/*                                                  */
/*   CS-454/654 Embedded Systems Development        */
/*   Instructor: Renato Mancuso <rmancuso@bu.edu>   */
/*   Boston University                              */
/*                                                  */
/*   Description: template file for serial          */
/*                communication server              */
/*                                                  */
/****************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "pc_crc16.h"
#include "lab3.h"
#include "lab3_troll.h"

#define GREETING_STR						\
    "CS454/654 - Final Project Server\n"				\
    "Authors: Eric Nohara-LeClair, Yin-Ching Lee, Jason Ha (BU)\n"				\

#define USAGE_STR							\
	"\nUSAGE: %s [-v] [-t percentage] <dev>\n"			\
	"   -v \t\t Verbose output\n"					\
	"   -t \t\t Invoke troll with specified bit flipping percentage\n" \
	"   -p \t\t Specify packet size for sending bytes\n" \
	"   -d \t\t Specify duration of LED byte flash\n" \
	"   <dev> \t Path to serial terminal device to use, e.g. /dev/ttyUSB0\n\n"

#define TROLL_PATH "./lab3_troll"

int main(int argc, char* argv[])
{
	double troll_pct=0.3;		// Perturbation % for the troll (if needed)
	int ifd,ofd,i,N,troll=0;	// Input and Output file descriptors (serial/troll)
	char opt;	// String input
	unsigned char ack;
	struct termios oldtio, tio;	// Serial configuration parameters
	int VERBOSE = 0;		// Verbose output - can be overriden with -v
	int dev_name_len, crc, attempts;
	char * dev_name = NULL;
	int PACKET_SIZE = MSG_BYTES_MSG; // default to full packet size
	int DURATION = 1; // default
	char * input_filename = "output";
	
	/* Parse command line options */
	while ((opt = getopt(argc, argv, "-t:vp:d:f:")) != -1) {
		switch (opt) {
		case 1:
			dev_name_len = strlen(optarg);
			dev_name = (char *)malloc(dev_name_len);
			strncpy(dev_name, optarg, dev_name_len);
			break;
		case 't':
			troll = 1; 
			troll_pct = atof(optarg);                    
			break;
		case 'v':
			VERBOSE = 1;
			break;
		case 'p':
			PACKET_SIZE = atoi(optarg);
			if (PACKET_SIZE < 1 || PACKET_SIZE > MSG_BYTES_MSG) {
				fprintf(stderr, "Packet size must be between 1 and %d\n", MSG_BYTES_MSG);
				exit(EXIT_FAILURE);
			}
			break;
		case 'd':
			DURATION = atoi(optarg);
			if (DURATION < 1 || DURATION > MAX_DURATION) {
				fprintf(stderr, "LED flash duration must be between 1 and %d\n", MAX_DURATION);
				exit(EXIT_FAILURE);
			}
			break;
		case 'f':
			input_filename = strdup(optarg);
			break;
		default:
			break;
		}
	}

	uint8_t packet[PACKET_SIZE]; // byte packet

	/* Check if a device name has been passed */
	if (!dev_name) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// Open the serial port (/dev/ttyS1) read-write
	ifd = open(dev_name, O_RDWR | O_NOCTTY);
	if (ifd < 0) {
		perror(dev_name);
		exit(EXIT_FAILURE);
	}

	printf(GREETING_STR);
	printf("\nUsing a packet size of %d bytes...\n", PACKET_SIZE);
	printf("Using an LED flash duration of %dms...\n\n", DURATION * 50);


	// Start the troll if necessary
	if (troll)
	{
		// Open troll process (lab5_troll) for output only
		FILE * pfile;		// Process FILE for troll (used locally only)
		char cmd[128];		// Shell command

		snprintf(cmd, 128, TROLL_PATH " -p%f %s %s", troll_pct,
			 (VERBOSE) ? "-v" : "", dev_name);

		pfile = popen(cmd, "w");
		if (!pfile) { perror(TROLL_PATH); exit(-1); }
		ofd = fileno(pfile);
	}
	else ofd = ifd;		// Use the serial port for both input and output
	
	/*
 	 * Set up the serial port
	 * Control modes (c_cflag):
	 *  - B2400:	9600 Baud
	 *  - CS8:	8 data bits
	 *  - CLOCAL:	Ignore modem control lines
	 *  - CREAD:	Enable receiving
	 *  1 stop bit, no parity, no flow control is default 
	 * Output modes (c_oflag):
	 *  defaults
	 * Input modes (c_iflag):
	 *  - IGNPAR:	Ignore parity errors
	 * Control characters (c_cc):
	 *  - VMIN:	Minimum recv length
	 */
	tcgetattr(ifd, &oldtio);
	tio.c_cflag 	= B9600 | CS8 | CLOCAL | CREAD;
	tio.c_iflag 	= IGNPAR;
	tio.c_oflag 	= 0;
	tio.c_lflag 	= 0;
	tio.c_cc[VMIN]	= 1;
	tcflush(ifd, TCIFLUSH);
	tcsetattr(ifd, TCSANOW, &tio);


	// open the audio bytes output file
	FILE *input_file = fopen(input_filename, "r");
	if (!input_file) {
		perror(input_filename);
		exit(EXIT_FAILURE);
	}

	// send a CONFIG packet to the board to coordinate
	ack = MSG_NACK;
	crc = 0xff, attempts=0;

	uint8_t config_packet[CONFIG_PACKET_SIZE];
	config_packet[0] = (uint8_t)PACKET_SIZE;
	config_packet[1] = (uint8_t)DURATION;

	N = CONFIG_PACKET_SIZE;
	crc = pc_crc16(config_packet, N);
	printf("Sending CONFIG packet with: %d bytes, crc: %x\n", N, crc);

	while (!ack) {
		printf("Sending config (attempt %d)...\n", ++attempts);

		uint8_t start_byte = 0x00;
		write(ofd, &start_byte, 1);

		for (i = MSG_BYTES_CRC - 1; i >= 0; i--) {
			uint8_t byte = (crc >> (8 * i)) & 0xFF;
			write(ofd, &byte, 1);
		}

		for (i = MSG_BYTES_MSG_LEN - 1; i >= 0; i--) {
			uint8_t byte = (N >> (8 * i)) & 0xFF;
			write(ofd, &byte, 1);
		}

		write(ofd, config_packet, N);

		printf("Config packet sent, waiting for ack... ");
		fflush(stdout);

		read(ifd, &ack, 1);
		printf("%s\n", ack ? "ACK" : "NACK, resending");
		fflush(stdout);
	}
	printf("\n");

	// send the data packets
	bool eof = false;
	int packet_num = 1;
	uint8_t byte;

	while(!eof)
	{
		ack = MSG_NACK;
		crc = 0xff, attempts=0;
		N = 0;

		// Read 10 lines (bytes) from file
		for (i = 0; i < PACKET_SIZE; i++) {
			if (fread(&byte, sizeof(uint8_t), 1, input_file) == 1) {
				packet[N++] = byte;
			} else {
				eof = true;
				break;
			}
		}
		
		// Compute crc (only lowest 16 bits are returned)
		crc = pc_crc16(packet, N);
		printf("Sending packet %d with: %d bytes, crc: %x\n", packet_num, N, crc);
		
		while (!ack)
		{
			printf("Sending (attempt %d)...\n", ++attempts);

			uint8_t start_byte = 0x00;
			write(ofd, &start_byte, 1); // Start byte

			// Send CRC
			for (i = MSG_BYTES_CRC - 1; i >= 0; i--) {
				uint8_t byte = (crc >> (8 * i)) & 0xFF;
				write(ofd, &byte, 1);
			}

			// Send message length
			for (i = MSG_BYTES_MSG_LEN - 1; i >= 0; i--) {
				uint8_t byte = (N >> (8 * i)) & 0xFF;
				write(ofd, &byte, 1);
			}

			// send the packet as raw bytes
			write(ofd, packet, N);
			
			printf("Message sent, waiting for ack... ");

			fflush(stdout);

			// Wait for MSG_ACK or MSG_NACK
			read(ifd, &ack, 1);
			printf("%s\n", ack ? "ACK" : "NACK, resending");
			fflush(stdout);
		}
		printf("\n");
		packet_num++;
	}

	// Send the END_OF_DATA message
	ack = MSG_NACK;
	crc = 0xff, attempts=0;

	uint8_t end_of_data[PACKET_SIZE];
	end_of_data[0] = END_OF_DATA_FLAG;
	for (i = 0; i < PACKET_SIZE - 1; i++) end_of_data[i + 1] = END_OF_DATA_PAD;
	N = PACKET_SIZE;

	crc = pc_crc16(end_of_data, N);
	printf("Sending END_OF_DATA packet with: %d bytes, crc: %x\n", N, crc);

	while (!ack) {
		printf("Sending (attempt %d)...\n", ++attempts);

		uint8_t start_byte = 0x00;
		write(ofd, &start_byte, 1); // Start byte

		// Send CRC
		for (i = MSG_BYTES_CRC - 1; i >= 0; i--) {
			uint8_t byte = (crc >> (8 * i)) & 0xFF;
			write(ofd, &byte, 1);
		}

		// Send message length
		for (i = MSG_BYTES_MSG_LEN - 1; i >= 0; i--) {
			uint8_t byte = (N >> (8 * i)) & 0xFF;
			write(ofd, &byte, 1);
		}

		// send the end of data packet as raw bytes
		write(ofd, end_of_data, N);
		
		printf("Message sent, waiting for ack... ");

		// Wait for MSG_ACK or MSG_NACK
		read(ifd, &ack, 1);
		printf("%s\n", ack ? "ACK" : "NACK, resending");
	}

	// Reset the serial port parameters
	tcflush(ifd, TCIFLUSH);
	tcsetattr(ifd, TCSANOW, &oldtio);

	// Close the serial port
	close(ifd);
	
	return EXIT_SUCCESS;
}

