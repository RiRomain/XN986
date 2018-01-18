/**
 *
 * SONiX SDK Example Code
 * Category: CRYPTO Codec (AES/DES/3DES/CRC)
 * File: snx_crypto_example.c
 * Usage: 
 *		 1. Execute snx_crypto 
 * NOTE:
 *		 Because the hardware limitation, the AES/DES/3DES and CRC share the same
 *		 HW resource. Only one function works at a time. 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>        
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "snx_crypto.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/
#define KEY_SIZE 24								/* the max key size */
#define BUFFERSIZE 32							/* Buffer size for AES/DES/3DES */

/* snx crypto configuration */
typedef struct snx_crypto_conf_s {
	enum crypto_mode mode;						/* AES/DES/3DES/CRC mode selection */
	enum crypto_mode op;						/* Encrypto or Decrypto in AES/DES/3DES mode */
	unsigned char key[KEY_SIZE];				/* Encrypto or Decrypto Key for AES/DES/3DES */
	unsigned int data_len;						/* Data length */
	void *buffer_in;							/* input buffer address */
	void *buffer_out;							/* output buffer address */
	int debug;									/* debug flag */
} snx_crypto_conf_t;

int fsize(FILE *fp);
int snx_crypto_flow (void *arg);

unsigned short sw_calc_crc16 (unsigned char *buf, unsigned int data_size);

