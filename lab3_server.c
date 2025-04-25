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
	"   <dev> \t Path to serial terminal device to use, e.g. /dev/ttyUSB0\n\n"

#define TROLL_PATH "./lab3_troll"

int main(int argc, char* argv[])
{
	double troll_pct=0.3;		// Perturbation % for the troll (if needed)
	int ifd,ofd,i,N,troll=0;	// Input and Output file descriptors (serial/troll)
	char opt;	// String input
	uint8_t buffer[MSG_BYTES_MSG]; // byte packet buffer
	struct termios oldtio, tio;	// Serial configuration parameters
	int VERBOSE = 0;		// Verbose output - can be overriden with -v
	int dev_name_len;
	char * dev_name = NULL;
	
	/* Parse command line options */
	while ((opt = getopt(argc, argv, "-t:v")) != -1) {
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
		default:
			break;
		}
	}

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
	FILE *input_file = fopen("output.txt", "r");
	if (!input_file) {
		perror("output.txt");
		exit(EXIT_FAILURE);
	}

	
	bool eof = false;
	char line[16]; // enough to hold "255\n\0"


	while(!eof)
	{
		unsigned char ack = MSG_NACK;
		int crc = 0xff, attempts=0;
		N = 0;
		
		for (i = 0; i < MSG_BYTES_MSG; i++) {
			buffer[i] = NULL;
		}


		// Read 10 lines (bytes) from file
		for (i = 0; i < MSG_BYTES_MSG; i++) {
			if (fgets(line, sizeof(line), input_file) == NULL) {
				eof = true;
				break;
			}

			// Convert string to integer and store in buffer
			int val = atoi(line);
			if (val < 0 || val > 255) {
				fprintf(stderr, "Invalid byte value: %s", line);
				break;
			}
		
			buffer[N++] = (uint8_t)val;
		}

		// convert byte array to char array
		char char_buffer[MSG_BYTES_MSG];
		for (i = 0; i < MSG_BYTES_MSG; i++) {
			char_buffer[i] = (char) buffer[i];
		}

		// Compute crc (only lowest 16 bits are returned)
		crc = pc_crc16(char_buffer, N);
		if (VERBOSE) printf("Sending %d bytes, crc: %x\n", N, crc);
		else printf("crc: %x\n", crc);
		
		while (!ack)
		{
			printf("Sending (attempt %d)...\n", ++attempts);
			
			// Send message
			dprintf(ofd, "%c", 0x0);			// Start byte
			

			for (i = MSG_BYTES_CRC-1; i >= 0; i--){	// CRC
				dprintf(ofd, "%c", crc >> (8*i));
			}
			for (i = MSG_BYTES_MSG_LEN-1; i >= 0; i--){	// Message length
				dprintf(ofd, "%c", N >> (8*i));
			}

			dprintf(ofd, "%s", char_buffer);
			
			printf("Message sent, waiting for ack... ");

			// Wait for MSG_ACK or MSG_NACK
			read(ifd, &ack, 1);
			printf("%s\n", ack ? "ACK" : "NACK, resending");
			
		}
		printf("\n");
	}

	// Reset the serial port parameters
	tcflush(ifd, TCIFLUSH);
	tcsetattr(ifd, TCSANOW, &oldtio);

	// Close the serial port
	close(ifd);
	
	return EXIT_SUCCESS;
}

