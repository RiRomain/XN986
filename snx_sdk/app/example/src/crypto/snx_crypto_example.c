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

#include "snx_crypto_flow.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

/* 
  This example reads from the specifiend PCM device 
  and writes to SD card for 10 seconds of data. 
 */  

static const char short_options[] = "ht:edf:k:p";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"type", required_argument, NULL, 't'},
	{"encrypto", no_argument, NULL, 'e'},
	{"decrypto", no_argument, NULL, 'd'},
    {"file_name", required_argument, NULL, 'f'},
	{"key", required_argument, NULL, 'k'},
	{"print", no_argument, NULL, 'p'},
	
    {0, 0, 0, 0}
};

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
		"Options:\n"
		"-h                 Print this message\n"
        "-t | --type        aes, des, 3des, crc \n"
		"-e | --encrypto    Encrypto operation in AES/DES/3DES mode (Default)\n"
		"-d | --decrypto    Decrypto operation in AES/DES/3DES mode\n"
		"-f | --file_name   The input filename\n"
		"-k | --key         Key in AES/DES/3DES mode\n"
		"-p | --print       Print the debug information\n"
		"NOTE: the input file of AES/DES/3DES should be 16-bytealigment, at least 32bytes\n"

		"", argv[0]);   
}

void generate_rand_key(unsigned char* key)
{
	int i;
	for(i = 0; i < KEY_SIZE; i++) {
		key[i] = rand() & 0xFF;
	}
}

int main(int argc, char *argv[])   
{
	snx_crypto_conf_t *snx_crypto_conf = NULL;	
	int print_flag =0;
	int file_flag = 0;
	int encrypto_flag=0, decrypto_flag=0;
	char *crypto_type = NULL;
	char *crypto_key = NULL;
	char *input = NULL;
	char *output = NULL;
	FILE *input_fd;
	FILE *output_fd;
	int rc = 0;
	size_t cached_size, read_size;
	
	snx_crypto_conf = malloc(sizeof(snx_crypto_conf_t));

	/*--------------------------------------------------------
		Option Value
	---------------------------------------------------------*/
	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   

		if (-1 == c)
			break;

		switch (c) {   
			case 0: /* getopt_long() flag */   
				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);   
			case 't':
				if (strlen(optarg) > 0) {
					crypto_type = malloc(strlen(optarg));
					strcpy(crypto_type, optarg);
				}
				break;
			case 'f':
				if (strlen(optarg) > 0) {
					input = malloc(strlen(optarg));
					output = malloc(strlen(optarg) + 10);
					strcpy(input, optarg);
					file_flag = 1;
				}
				break;
			case 'p':
				print_flag = 1;
				snx_crypto_conf->debug = 1;
				break;
			case 'e':
				if ((encrypto_flag == 0) && (decrypto_flag == 0)) {
					encrypto_flag = 1;
				} else {
					printf("[SNX-CRYPTO] Do not execute -e and -d at the same time.\n");
					goto EXIT;
				}
				break;
			case 'd':
				if ((encrypto_flag == 0) && (decrypto_flag == 0)) {
					decrypto_flag = 1;
				} else {
					printf("[SNX-CRYPTO] Do not execute -e and -d at the same time.\n");
					goto EXIT;
				}
				break;
			case 'k':
				if (strlen(optarg) > KEY_SIZE) {
					printf("[SNX-CRYPTO] key length should not be over %d byte.\n", KEY_SIZE);
					goto EXIT;
				}
				if (strlen(optarg) > 0) {
					crypto_key = malloc(strlen(optarg));
					strcpy(crypto_key, optarg);
				}
				break;
			default:
				usage(stderr, argc, argv);
				goto EXIT;
		}
	}
	
	/*--------------------------------------------------------
		Config setup
	---------------------------------------------------------*/

	if (crypto_type) {
		int input_file_size;
		int buffer_size;
	
		if(file_flag) {
			
			input_fd = fopen(input,"rb");
			if (input_fd <=0 ) {
				printf("[SNX-CRYPTO] Open input file failed\n");
				goto EXIT;
			}
			
			input_file_size = fsize(input_fd);
		}
		
		/*--------------------------------------------------------
			Mode selection
		---------------------------------------------------------*/
	
		if(!strcasecmp(crypto_type, "aes"))
		{
			buffer_size  = BUFFERSIZE;
			snx_crypto_conf->mode = SNX_CRYPTO_MODE;
			if(decrypto_flag)
				snx_crypto_conf->op = SNX_AES_DECRYPT;
			else
				snx_crypto_conf->op = SNX_AES_ENCRYPT;
		} else if(!strcasecmp(crypto_type, "des"))
		{
			buffer_size  = BUFFERSIZE;
			snx_crypto_conf->mode = SNX_CRYPTO_MODE;
			if(decrypto_flag)
				snx_crypto_conf->op = SNX_DES_DECRYPT;
			else
				snx_crypto_conf->op = SNX_DES_ENCRYPT;
		} else if(!strcasecmp(crypto_type, "3des"))
		{
			buffer_size  = BUFFERSIZE;
			snx_crypto_conf->mode = SNX_CRYPTO_MODE;
			if(decrypto_flag)
				snx_crypto_conf->op = SNX_3DES_DECRYPT;
			else
				snx_crypto_conf->op = SNX_3DES_ENCRYPT;
		} else if(!strcasecmp(crypto_type, "crc"))
		{
			buffer_size  = input_file_size;
			snx_crypto_conf->mode = SNX_CRC_MODE;
		} else {
			printf("[SNX-CRYPTO] Wrong crypto operation\n");
			goto EXIT;
		}
		
		if (input_fd) {
		
			// allocate input data buffer
			snx_crypto_conf->buffer_in = (void *)malloc(buffer_size);
		
			if (snx_crypto_conf->mode == SNX_CRYPTO_MODE) {
			
				if (input_file_size & 0xf) {
					printf("[SNX-CRYPTO] input data should be 16-bytes alignment : %d\n", input_file_size);
					goto EXIT;
				}
				
				// Open the output data file.
				sprintf(output, "%s_%s",input, crypto_type);
				output_fd = fopen(output,"wb");
				if (output_fd <=0 ) {
					printf("[SNX-CRYPTO] Open output file failed\n");
					goto EXIT;
				}
				
				if(!crypto_key) {
					printf("[SNX-CRYPTO] Please enter the key\n");
					goto EXIT;
				}
				strcpy(snx_crypto_conf->key, crypto_key);
				
				// allocate output data buffer
				snx_crypto_conf->buffer_out = (void *)malloc(buffer_size);
				
			} else if (snx_crypto_conf->mode == SNX_CRC_MODE) {
			
				if (input_file_size & 0x3) {
					printf("[SNX-CRYPTO] input data should be 4-bytes alignment : %d\n", input_file_size);
					goto EXIT;
				}
				// allocate output data buffer for print
				snx_crypto_conf->buffer_out = (void *)malloc(sizeof(unsigned short));
			
			}
			
			cached_size = 0;
			read_size = 0;
			
			while (!feof(input_fd)) {

				read_size = fread((char*)(snx_crypto_conf->buffer_in + cached_size) , 1, (buffer_size - cached_size), input_fd);
				
				if(read_size > 0) {
					cached_size = cached_size + read_size;
					if(print_flag)
						printf("[SNX-CRYPTO] read %d bytes (%d) from %s\n", read_size, cached_size, input);
				}
				
				if(cached_size == buffer_size) {
				
					snx_crypto_conf->data_len = buffer_size;
				
					rc = snx_crypto_flow(snx_crypto_conf);
					if (rc < 0 ) {
						printf("[SNX-CRYPTO] snx_crypto_flow error !\n");
						break;
					}
									
					if (snx_crypto_conf->mode == SNX_CRYPTO_MODE) {
						fwrite(snx_crypto_conf->buffer_out,  snx_crypto_conf->data_len, 1, output_fd);
						
					} else if (snx_crypto_conf->mode == SNX_CRC_MODE) {
						unsigned short sw_crc16 = 0;
						sw_crc16 = sw_calc_crc16((char *)snx_crypto_conf->buffer_in, snx_crypto_conf->data_len);
						printf("[SNX-CRYPTO] Total data size : %d, HWCRC: 0x%x, SWCRC: 0x%x\n",input_file_size, *((unsigned short*)snx_crypto_conf->buffer_out), sw_crc16);
					}
					cached_size = 0;
				}
			}
			
			// flush the data to calculate in AES/DES/3DES mode
			if ((cached_size) && (snx_crypto_conf->mode == SNX_CRYPTO_MODE)) {
				snx_crypto_conf->data_len = cached_size;
				rc = snx_crypto_flow(snx_crypto_conf);
				if (rc < 0 ) {
					printf("[SNX-CRYPTO] snx_crypto_flow error !\n");
				}
					
				fwrite(snx_crypto_conf->buffer_out,  snx_crypto_conf->data_len, 1, output_fd);
			}
			
			/* Free and close the buffers and files */
			free(snx_crypto_conf->buffer_out);
			free(snx_crypto_conf->buffer_in);
			if (output_fd)
				fclose(output_fd);
			fclose(input_fd);
			
		} else {
			printf("[SNX-CRYPTO] Please enter the input file name\n");
			goto EXIT;
		}
	} else {
	
			printf("[SNX-CRYPTO] Please enter the crypto type (aes/des/3des/crc)\n");
			goto EXIT;
	}
		
EXIT:
	if(input) {
		free(input);
	}
	if(output) {
		free(input);
	}
	if(crypto_key) {
		free(crypto_key);
	}
	if(crypto_type) {
		free(crypto_type);
	}
	if(snx_crypto_conf)
		free(snx_crypto_conf);

    return 0;
}
