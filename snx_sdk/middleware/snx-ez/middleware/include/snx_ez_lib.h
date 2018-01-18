

#ifndef __SNX_LIBEZ_H__
#define __SNX_LIBEZ_H__

#ifdef __cplusplus
extern "C" {
#endif


#define FILE_BUF_LEN		2048

#define snx_debug(fmt, args...)		fprintf(stderr, "[<DBG><%s>]::" fmt, __func__, ## args)
#define snx_info(fmt, args...)		fprintf(stderr, "[<INFO>]::" fmt, ## args)
#define snx_error(fmt, args...)		fprintf(stderr, "[<ERR>]::" fmt, ## args)


#define WIFI_ENCRYPT 2


#define GPIO_PIN_0		0
#define GPIO_PIN_1		1
#define GPIO_PIN_2		2
#define GPIO_PIN_3		3

#define CMD_RENEW		"udhcpc_renew.sh"
#define RENEW_PING		"www.yahoo.com.tw"
#define RENEW_PID_FILE		"/var/run/udhcpc.wlan0.pid"

#define CONF_DEF		"default.conf"

#if WIFI_ENCRYPT == 0		// WEP
#define CONF_AP			"hostapd_wep.conf"
#elif WIFI_ENCRYPT == 1		// WPA
#define CONF_AP			"hostapd_wpa.conf"
#else				// WPA2
#define CONF_AP			"hostapd_wpa2.conf"
#endif  // WIFI_ENCRYPT

#define CONF_STA		"wpa_supplicant.conf"

#define CONF_DHCPD		"udhcpd.conf"

#define SNIP39_UID_FILE		"SNIP39_UID.conf"


#define GALAXY_PID_FILE		"/var/run/sonix-proj.pid"
#define EZ_PID_FILE		"/var/run/ez.pid"
#define CSTREAM_PID_FILE	"/var/run/cstreamer.pid"


#define WIFI_DEV		"wlan0"
#define ETH_DEV			"eth0"


#if MT7601==1
#define WIFI_DRV		"mt7601Usta"
#elif RT3070==1
#define WIFI_DRV		"rt3070sta"
#elif BCM==1
#define WIFI_DRV		"bcmdhd"
#else // RTL 8188
#define WIFI_DRV		"8188eu"
#endif


#if MT7601==1
#define WIFI_AP_DRV		"mt7601Uap"
#elif RT3070==1
#define WIFI_AP_DRV		"rtnet3070ap"
#elif BCM==1
#define WIFI_AP_DRV		"bcmdhd"
#else // RTL 8188
#define WIFI_AP_DRV		"8188eu"
#endif


#define DETECT_USB		"/proc/bus/usb/001/002"
#define DETECT_WIFI		"/proc/sys/net/ipv4/conf/"

#define CMD_COPY		"cp -f"

#define CMD_AP			"hostapd -B"
#define CMD_STA			"wpa_supplicant -B -Dwext "
//#define CMD_DHCPC		"udhcpc -R -n -p /var/run/udhcpc.wlan0.pid -i "
#define CMD_DHCPC		"udhcpc -p /var/run/udhcpc.wlan0.pid -i "
#define CMD_DHCPD		"udhcpd"


#define CMD_EZ			"snx_ez "
#define CMD_TUNNELING		"http-tunneling-serv "
#define CMD_PROJ		"sonix-proj "
#define CMD_TWOWAY		"twowayaudio "

#define	SN98600_WIFI_INFO	"wifi_info"

#define GFWVER			"/usr/bin/gfwver"
#define FW_VERSION_SIZE		"64"
#define FW_VERSION_SIZE_NUM 	64

// ez_setup status
#define EZ_SETUP_NONE		(0)
#define EZ_SETUP_GPIO_START	(1)
#define EZ_SETUP_GPIO_END	(2)

#define PCM_PLAY		"/usr/bin/pcm_play"

/* audio data */
#define START_PCM		"/etc/notify/start.pcm"
#define ONLINE_PCM		"/etc/notify/online.pcm"
#define OFFLINE_PCM		"/etc/notify/offline.pcm"
#define RESET_PCM		"/etc/notify/reset.pcm"
#define QRSCAN_PCM		"/etc/notify/qrcode.pcm"
#define VIEW_PCM		"/etc/notify/view.pcm"
#define RETRY_PCM		"/etc/notify/retry.pcm"

#define SCAN_WIFI_FILE		"/etc/pre_st_srvy"


#define	SIZE 32

enum {
	NONWRITE	= 0, // NOT WRITE TO FILE
	WRITE		= 1,
} RW_METHOD;


enum {
	EZ_METHOD_WIFI	= 1,
	EZ_METHOD_QR	= 2,
	EZ_METHOD_TONE	= 4,
} EZ_METHOD;

enum {
	EZ_MODE_NONE,		// 0
	EZ_MODE_QUIT,		// 1
	EZ_MODE_SCAN,		// 2
	EZ_MODE_STA	= 4,	// 4
	EZ_MODE_WIFI	= 8,	// 8
	EZ_MODE_QR	= 16,	// 16
} EZ_MODE;


typedef void	(*snx_cmd_cb)(const char *);


extern void	encrypt_string(char *text, char *encryptText, int len);
extern void	decrypt_string(char *encryptText, char *text, int len);

extern int	snx_get_mode(void);
extern void	snx_set_mode(int ivalue);


extern int	snx_write_nvram(char *id, char *data);
extern int	snx_read_nvram(char *id, char *data, int len);
extern void	snx_read_uid(char **uid, int len);
extern void	snx_write_uid(char *uid);

extern void	snx_kill_ez(void);

extern void	snx_set_ssid(char *ssid_1,char **ssid_key, int write);
extern void	snx_set_uid(char *uid);

extern char	*base64_decode(const char *data, int data_len,int *len);

extern void	snx_play_audio(char *src_audio);

extern void	aes_cbc_encrypt(unsigned char* in, int inl, unsigned char *out, int* len, char * key);
extern void	aes_cbc_decrypt(unsigned char* in, int inl, unsigned char *out, char *key);

extern int	fileExists(const char* file);


extern int	snx_scan_check(char *dev, int method, snx_cmd_cb cmd_cb);
extern void	snx_wifi_dev_rdy(void);
extern int	snx_wifi_driver_check(void);

#ifdef __cplusplus
}
#endif
#endif //__SNX_LIBEZ_H__
