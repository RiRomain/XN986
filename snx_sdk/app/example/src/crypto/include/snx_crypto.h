/** \file snx_crypto.h
 * Sonix ENC_DEC driver header file
 * \n Define the interface of user space.
 * \author chao_zhang
 * \date   2012/10/11; 2013/11/12 merge crc & crypto
 */

#ifndef SN926_ENC_DEC_H
#define SN926_ENC_DEC_H

enum crypto_mode
{
	SNX_CRC_MODE,
	SNX_CRYPTO_MODE,
	SNX_AES_ENCRYPT,
	SNX_AES_DECRYPT,
	SNX_DES_ENCRYPT,
	SNX_DES_DECRYPT,
	SNX_3DES_ENCRYPT,
	SNX_3DES_DECRYPT,
	SNX_CRC_REG,
	SNX_CRC_DRAM,
};

/** \struct sonix_enc_dec_info
 * \brief Encrypt/Decrypt information of ENC_DEC
 * \n
 * \n data_len:The length of the data to be encrypted/decrypted
 * \n mode:ENC_DEC device mode
 * \n key:enc/dec key value
 *
 */
struct snx_crypto_info
{
	enum crypto_mode mode;
	unsigned int data_len;
	unsigned int key[6];
};

/** \struct snx_crc_info
 * \brief crc information 
 * \n
 * \n data_len:The length of the data to be calculate
 * \n crc:ENC_DEC device mode
 *
 */
struct snx_crc_info
{
	unsigned int data_len;
	unsigned short crc;
};

/**
 * \defgroup SN926_ENC_DEC_H_G1 The ioctl command of ENC_DEC driver
 * @{
 */
#define SNX_CRYPTO_MAGIC		'c' /*!< Magic number of the CRYPTO driver */
#define SNX_CRYPTO_GMAX_DATA_SIZE	_IOR(SNX_CRYPTO_MAGIC, 1, unsigned int) /*!< Get the maxinum data length of one time encrypted/decrypted data */
#define SNX_INIT_BUF			_IOR(SNX_CRYPTO_MAGIC, 1, struct snx_crypto_info) /*!< init data buf */
#define SNX_CRYPTO_CRYPT		_IOWR(SNX_CRYPTO_MAGIC, 2, struct snx_crypto_info) /*!< Enable the encrypt/decrypt */
#define SNX_CRC_CALCULATE		_IOWR(SNX_CRYPTO_MAGIC, 2, struct snx_crc_info) /*!< calculate crc */
/** @} */

#endif
