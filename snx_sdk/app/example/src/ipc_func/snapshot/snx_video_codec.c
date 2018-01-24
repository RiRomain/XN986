/**
 *
 * SONiX SDK Example Code
 * Category: Video Encode
 * File: video_codec.c
 *
 * NOTE:
 *
 */

#include "snx_video_codec.h"

#include "generated/snx_sdk_conf.h"
/*
	Before capture stream starts, make sure one m2m stream is started.
	The capture stream is related to the m2m stream, including
	frame rate
	scaling down (1 1/2 1/4)
	format (H264, MJPEG, RAW)
*/

char ascii_2_font[] =
{
	// ascii , shift
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x00 Can't display
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x08 Can't display
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x10 Can't display
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x18 Can't display
	0x24, 0x3F, 0x3C, 0x3D, 0x3A, 0x3E, 0x40, 0x3B,	//0x20
	0x2D, 0x2E, 0x41, 0x42, 0x26, 0x2A, 0x43, 0x28,	//0x28
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	//0x30
	0x08, 0x09, 0x27, 0x38, 0x31, 0x25, 0x32, 0x37,	//0x38

	0x36, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,	//0x40
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,	//0x48
	0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,	//0x50
	0x21, 0x22, 0x23, 0x33, 0x35, 0x34, 0x39, 0x2B,	//0x58
	0x3B, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,	//0x60
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,	//0x68
	0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,	//0x70
	0x21, 0x22, 0x23, 0x2F, 0x2C, 0x30, 0x29, 0x24,	//0x78

};

unsigned int  osd_font[] =
{
	//////////////////// 0 ////////////////////
	0X00000000, 0X007C0000, 0X00E600C6, 0X00DE00F6,
	0X00C600CE, 0X007C00C6, 0X00000000, 0X00000000,
	//////////////////// 1 ////////////////////
	0X00000000, 0X00300000, 0X003C0038, 0X00300030,
	0X00300030, 0X00FC0030, 0X00000000, 0X00000000,
	//////////////////// 2 ////////////////////
	0X00000000, 0X007C0000, 0X00C000C6, 0X00300060,
	0X000C0018, 0X00FE00C6, 0X00000000, 0X00000000,
	//////////////////// 3 ////////////////////
	0X00000000, 0X007C0000, 0X00C000C6, 0X007800C0,
	0X00C000C0, 0X007C00C6, 0X00000000, 0X00000000,
	//////////////////// 4 ////////////////////
	0X00000000, 0X00600000, 0X00780070, 0X0066006C,
	0X006000FE, 0X00F00060, 0X00000000, 0X00000000,
	//////////////////// 5 ////////////////////
	0X00000000, 0X00FE0000, 0X00060006, 0X007E0006,
	0X00C000C0, 0X007C00C6, 0X00000000, 0X00000000,
	//////////////////// 6 ////////////////////
	0X00000000, 0X00380000, 0X0006000C, 0X007E0006,
	0X00C600C6, 0X007C00C6, 0X00000000, 0X00000000,
	//////////////////// 7 ////////////////////
	0X00000000, 0X00FE0000, 0X00C000C6, 0X00300060,
	0X00180018, 0X00180018, 0X00000000, 0X00000000,
	//////////////////// 8 ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X007C00C6,
	0X00C600C6, 0X007C00C6, 0X00000000, 0X00000000,
	//////////////////// 9 ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X00FC00C6,
	0X00C000C0, 0X003C0060, 0X00000000, 0X00000000,
	//////////////////// A ////////////////////
	0X00000000, 0X00100000, 0X006C0038, 0X00C600C6,
	0X00C600FE, 0X00C600C6, 0X00000000, 0X00000000,
	//////////////////// B ////////////////////
	0X00000000, 0X007E0000, 0X00CC00CC, 0X007C00CC,
	0X00CC00CC, 0X007E00CC, 0X00000000, 0X00000000,
	//////////////////// C ////////////////////
	0X00000000, 0X00780000, 0X008600CC, 0X00060006,
	0X00860006, 0X007800CC, 0X00000000, 0X00000000,
	//////////////////// D ////////////////////
	0X00000000, 0X003E0000, 0X00CC006C, 0X00CC00CC,
	0X00CC00CC, 0X003E006C, 0X00000000, 0X00000000,
	//////////////////// E ////////////////////
	0X00000000, 0X00FE0000, 0X008C00CC, 0X003C002C,
	0X008C002C, 0X00FE00CC, 0X00000000, 0X00000000,
	//////////////////// F ////////////////////
	0X00000000, 0X00FE0000, 0X008C00CC, 0X003C002C,
	0X000C002C, 0X001E000C, 0X00000000, 0X00000000,
	//////////////////// G ////////////////////
	0X00000000, 0X00780000, 0X008600CC, 0X00060006,
	0X00C600F6, 0X003E006C, 0X00000000, 0X00000000,
	//////////////////// H ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00FE00C6,
	0X00C600C6, 0X00C600C6, 0X00000000, 0X00000000,
	//////////////////// I ////////////////////
	0X00000000, 0X003C0000, 0X00180018, 0X00180018,
	0X00180018, 0X003C0018, 0X00000000, 0X00000000,
	//////////////////// J ////////////////////
	0X00000000, 0X00F00000, 0X00600060, 0X00600060,
	0X00660060, 0X003C0066, 0X00000000, 0X00000000,
	//////////////////// K ////////////////////
	0X00000000, 0X00CE0000, 0X006C00CC, 0X003C006C,
	0X006C006C, 0X00CE00CC, 0X00000000, 0X00000000,
	//////////////////// L ////////////////////
	0X00000000, 0X001E0000, 0X000C000C, 0X000C000C,
	0X008C000C, 0X00FE00CC, 0X00000000, 0X00000000,
	//////////////////// M ////////////////////
	0X00000000, 0X00C60000, 0X00FE00EE, 0X00C600D6,
	0X00C600C6, 0X00C600C6, 0X00000000, 0X00000000,
	//////////////////// N ////////////////////
	0X00000000, 0X00C60000, 0X00DE00CE, 0X00F600FE,
	0X00C600E6, 0X00C600C6, 0X00000000, 0X00000000,
	//////////////////// O ////////////////////
	0X00000000, 0X00380000, 0X00C6006C, 0X00C600C6,
	0X00C600C6, 0X0038006C, 0X00000000, 0X00000000,
	//////////////////// P ////////////////////
	0X00000000, 0X007E0000, 0X00CC00CC, 0X007C00CC,
	0X000C000C, 0X001E000C, 0X00000000, 0X00000000,
	//////////////////// Q ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X00C600C6,
	0X00F600D6, 0X0060007C, 0X000000E0, 0X00000000,
	//////////////////// R ////////////////////
	0X00000000, 0X007E0000, 0X00CC00CC, 0X007C00CC,
	0X00CC006C, 0X00CE00CC, 0X00000000, 0X00000000,
	//////////////////// S ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X0038000C,
	0X00C60060, 0X007C00C6, 0X00000000, 0X00000000,
	//////////////////// T ////////////////////
	0X00000000, 0X00FF0000, 0X00180099, 0X00180018,
	0X00180018, 0X003C0018, 0X00000000, 0X00000000,
	//////////////////// U ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00C600C6,
	0X00C600C6, 0X007C00C6, 0X00000000, 0X00000000,
	//////////////////// V ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00C600C6,
	0X006C00C6, 0X00100038, 0X00000000, 0X00000000,
	//////////////////// W ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00C600C6,
	0X00FE00D6, 0X00C600EE, 0X00000000, 0X00000000,
	//////////////////// X ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X0038006C,
	0X00C6006C, 0X00C600C6, 0X00000000, 0X00000000,
	//////////////////// Y ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X0038006C,
	0X00380038, 0X007C0038, 0X00000000, 0X00000000,
	//////////////////// Z ////////////////////
	0X00000000, 0X00FE0000, 0X006200C6, 0X00180030,
	0X0086000C, 0X00FE00C6, 0X00000000, 0X00000000,
	//////////////////// S ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// = ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X0000007E,
	0X007E0000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// , ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00180000, 0X00180018, 0X0000000C, 0X00000000,
	//////////////////// : ////////////////////
	0X00000000, 0X00000000, 0X001C003C, 0X00000000,
	0X00000000, 0X001C003C, 0X00000000, 0X00000000,
	//////////////////// / ////////////////////
	0X00000000, 0X03800000, 0X01C00180, 0X00E000C0,
	0X00700060, 0X00380030, 0X001C0018, 0X00000000,
	//////////////////// ~ ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X31F800F0,
	0X0E001F0C, 0x00000000, 0x00000000, 0x00000000,
	//////////////////// - ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X000003FE,
	0X00000000, 0X00000000, 0X00000000, 0x00000000,
	//////////////////// _ ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0x7FFF0000, 0x00007FFF, 0x00000000,
	//////////////////// | ////////////////////
	0X00600060, 0X00600060, 0X00600060, 0X00600060,
	0X00600060, 0X00600060, 0X00600060, 0X00600060,
	//////////////////// ( ////////////////////
	0X00F000C0, 0X00180038, 0X000C001C, 0X000E000E,
	0X000E000E, 0X001C001C, 0X00380018, 0X00E00070,
	//////////////////// ) ////////////////////
	0X001C000E, 0X00700038, 0X00E000E0, 0X00E000E0,
	0X00E000E0, 0X006000E0, 0X00380070, 0X000E001C,
	//////////////////// { ////////////////////
	0X00C001C0, 0X00E000E0, 0X00E000E0, 0X006000E0,
	0X00E000F0, 0X00E000E0, 0X00E000E0, 0X038000C0,
	//////////////////// } ////////////////////
	0X01C000E0, 0X01C001C0, 0X01C001C0, 0X038001C0,
	0X01C003C0, 0X01C001C0, 0X01C001C0, 0X00E001C0,
	//////////////////// < ////////////////////
	0X00000000, 0X00E000C0, 0X001C0070, 0X000F000E,
	0X0038001C, 0X00E00070, 0X000000C0, 0X00000000,
	//////////////////// > ////////////////////
	0X00000000, 0X00070003, 0X0038001C, 0X00F00070,
	0X001C0038, 0X0007000E, 0X00000003, 0X00000000,
	//////////////////// [ ////////////////////
	0X0003001F, 0X00030003, 0X00030003, 0X00030003,
	0X00030003, 0X00030003, 0X00030003, 0X0000001F,
	//////////////////// ] ////////////////////
	0X0018001F, 0X00180018, 0X00180018, 0X00180018,
	0X00180018, 0X00180018, 0X00180018, 0X0000001F,
	//////////////////// \ ////////////////////
	0X00000000, 0X0038001C, 0X00700038, 0X00E00070,
	0X01C000E0, 0X03800180, 0X03000300, 0X00000000,
	//////////////////// @ ////////////////////
	0X00000000, 0X019C01F8, 0X03FE038C, 0X03FE03FE,
	0X01FE03FE, 0X01FE03FE, 0X01DC000C, 0X000000F8,
	//////////////////// ? ////////////////////
	0X00000000, 0X01C700FE, 0X01EE01CF, 0X007000E0,
	0X00180038, 0X00000000, 0X003C003C, 0X00000000,
	//////////////////// ; ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00380038,
	0X00000000, 0X00180000, 0X0030003C, 0X00180038,
	//////////////////// ^ ////////////////////
	0X00700000, 0X018C00D8, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// $ ////////////////////
	0X00300000, 0X00FC00F8, 0X003601B6, 0X0078003C,
	0X01B000F0, 0X01B601B0, 0X007C00FC, 0X00000030,
	//////////////////// ` ////////////////////
	0X000C0000, 0X00000018, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// " ////////////////////
	0X01B00000, 0X00D800D8, 0X000000D8, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// # ////////////////////
	0X06C00000, 0X06C006C0, 0X0FF006C0, 0X03600360,
	0X03600360, 0X01B007F8, 0X01B001B0, 0X000001B0,
	//////////////////// % ////////////////////
	0X00000000, 0X03F00660, 0X01F803D8, 0X00C001B0,
	0X036000C0, 0X06F007E0, 0X019803F0, 0X00000000,
	//////////////////// ! ////////////////////
	0X00000000, 0X00C000C0, 0X00C000C0, 0X00C000C0,
	0X00C000C0, 0X000000C0, 0X00C00000, 0X000000C0,
	//////////////////// & ////////////////////
	0X00000000, 0X00380000, 0X006C006C, 0X0018003C,
	0X00FC01FC, 0X00E600F6, 0X01E60066, 0X000000FC,
	//////////////////// * ////////////////////
	0X00000000, 0X00000000, 0X00300000, 0X007800FC,
	0X00780030, 0X003000FC, 0X00000000, 0X00000000,
	//////////////////// + ////////////////////
	0X00000000, 0X00000000, 0X00180018, 0X00180018,
	0X001800FF, 0X00180018, 0X00000018, 0X00000000,
	//////////////////// . ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X000C0000, 0X0000000C
};

/*
	M2M + Cap + Rc : Integrate M2M and Capture stream flow with Rate Control
*/
void snx_m2m_cap_rc_flow(void *arg)
{
	long long unsigned int data_size = 0;					//Count the total datasize
	stream_conf_t *stream = (stream_conf_t *)arg;
	struct snx_m2m *m2m = &stream->m2m;
	char* outputpath = stream->outputpath;
	struct snx_rc *rc = NULL;			//rate control use

	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;

	int count_num = 0;					//Encoded frame count
	int real_fps = 0;					//real_fps frame count
	int ret;
	int yuv_rate = 0;
	int yuv_count = 0;

	int frame_num = stream->frame_num;
	char filename[120];
	char filename_temp[120];
	char OSD_text[120];
	char syscmd[120];

	unsigned char * pYUV420 = NULL;
	if(m2m->codec_fmt == V4L2_PIX_FMT_SNX420) {
		pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);
		yuv_rate = (m2m->isp_fps/stream->yuv_frame) -1;
	}
#if 0
	if (m2m->m2m == 0) {
		if (stream->mutex) {
			/* Wait for the m2m stream has been start */
			pthread_mutex_lock(stream->mutex);
			(*stream->ref)++;
			pthread_cond_wait(stream->cond, stream->mutex);
			pthread_mutex_unlock(stream->mutex);
		}
	}
#endif

	printf("============snx_m2m_cap_rc_flow============\n");

	if(m2m->m2m) {
		/* Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);

		/* Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* Start ISP */
		snx_isp_start(m2m);
	}

	/* Open Video Encode Device */
	m2m->codec_fd = snx_open_device(m2m->codec_dev);

	/* Initialize Video Encode */
	ret = snx_codec_init(m2m);
	if(ret != 0) goto err_init;

	/* Set Codec GOP */
	snx_codec_set_gop(m2m);


	/* Bitrate Rate Control is only support for H264 */
	if((m2m->bit_rate) && (m2m->codec_fmt == V4L2_PIX_FMT_H264)) {

		rc = malloc(sizeof(struct snx_rc));

		/* Initialize rate control arguments */
		rc->width = m2m->width;				//Bit-rate control width
		rc->height = m2m->height;			//Bit rate control height
		rc->codec_fd = m2m->codec_fd;		//point to the codec fd
		rc->Targetbitrate = m2m->bit_rate;  //rate control target bitrate
		rc->framerate = m2m->codec_fps;		//point to the codec frame rate
		rc->gop = m2m->gop;					//codec gop
		//rc->reinit = 1;
		/*Initialize rate control */
		m2m->qp = snx_codec_rc_init(rc, SNX_RC_INIT);
	}

	// Set QP for MJPEG
	if (m2m->codec_fmt == V4L2_PIX_FMT_MJPEG)
		snx_codec_set_qp(m2m, V4L2_CID_JPEG_COMPRESSION_QUALITY);

	/* Start Video Encode */
	ret = snx_codec_start(m2m);
	if(ret != 0) goto err_start;

	/* Data Stamp */
	//if(strlen(stream->cds.dev_name))
	//	snx_vc_data_stamp(DS_SET_ALL, (void *)&stream->cds);

	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;

	while(stream->live){
		/* Read from Video Codec */
		ret = snx_codec_read(m2m);
		/* Check if any frame encodec */
		if(m2m->cap_bytesused != 0) {
			count_num ++;
			real_fps ++;
			gettimeofday(&tv,NULL);
			end_t = tv.tv_sec * 1000000 + tv.tv_usec;
			period = end_t - start_t;

			data_size += m2m->cap_bytesused;

			if (stream->debug != 0) {
				if (period - pre_period >= 1000000) {
					pre_period = period;

					printf("snx_record %d x %d Real fps = %d,(real_frames= %d)\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale)
					, real_fps
					,count_num);

					printf("Datasize %lld bytes,BitRate %lld Kbps  QP == %d\n"
					, data_size
					, (data_size>>7)
					, m2m->qp);

					real_fps = 0;
					data_size = 0;
				}
			}
			// TODO
			sprintf(OSD_text,"test %d",count_num);
			//snx_cds_set_datastamp(stream->cds.dev_name, OSD_text, sizeof(OSD_text));
			/*
				Bit Rate Control Flow
				Update the QP of the next frame to keep the bitrate. (CBR).
			*/
			if((m2m->bit_rate) && (m2m->codec_fmt == V4L2_PIX_FMT_H264)) {
				m2m->qp = snx_codec_rc_update(m2m, rc);
			        snx_md_drop_fps(rc, &m2m->force_i_frame);
			}


			/* Handle the encoded frame */
			if((frame_num > 0) && (stream->state == 1)) {
					int has_written = 0;
					int leng;
					int size =0;
					memset(syscmd, 0x00, sizeof(filename));
					memset(syscmd, 0x00, sizeof(syscmd));
					unsigned char * target_ptr = NULL;

					if(m2m->codec_fmt != V4L2_PIX_FMT_SNX420) {
						size =m2m->cap_bytesused;
						target_ptr = m2m->cap_buffers[m2m->cap_index].start;
						// TODO
						sprintf(filename_temp, "%s/snapshot_t.jpg", outputpath);
						sprintf(filename, "%s/snapshot.jpg", outputpath);
					}
					else {	//if(pYUV420)
						if (yuv_rate == yuv_count) {
							if (stream->y_only)
								size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale));
							else
								size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;

							ret = snx_420line_to_420(m2m->cap_buffers[m2m->cap_index].start, (char *)pYUV420
								, (m2m->width/m2m->scale)
								, (m2m->height/m2m->scale));
							target_ptr = pYUV420;

							if (stream->y_only)
								sprintf(filename, "%s/snapshot_%u_%u.y",
								outputpath, (unsigned int)tv.tv_sec,(unsigned int)tv.tv_usec);
							else
								sprintf(filename, "%s/snapshot_%u_%u.yuv",
								outputpath, (unsigned int)tv.tv_sec,(unsigned int)tv.tv_usec);
								yuv_count =0;
						} else {
							target_ptr = NULL;
							yuv_count++;
						}
					}


					if (target_ptr) {
						printf("SNAPSHOT....\n");
						stream->fd = open(filename_temp, O_RDWR | O_NONBLOCK | O_CREAT);
						while(1){
							leng = write(stream->fd, target_ptr + has_written, size - has_written);
							if(leng <= 0);
							else if(leng <  size - has_written){
								has_written += leng;
							}else  if(leng ==  size - has_written){
								break;
							}else{
								printf("Write error");
								break;
							}
						}
						// TODO move filename
						rename(filename_temp,filename);
						//sprintf(syscmd, "echo %s >> %s/snaplist.txt",filename, outputpath);
						//system(syscmd);
						close(stream->fd);
						frame_num--;	//recording
					}
			}  else if((frame_num == 0) && (stream->state == 1)) {
				memset(syscmd, 0x00, sizeof(syscmd));
				printf("Snapshot Done\n");
				//sprintf(syscmd, "cat %s/snaplist.txt >> /dev/console", outputpath);
				//system(syscmd);
				stream->state = 0;
				frame_num = stream->frame_num;
			} else {
				frame_num = stream->frame_num;
				stream->state = 0;
			}
		} //if(m2m->cap_bytesused != 0)
		/* Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);
	}

	/* To finish the m2m stream properly, call the apis in order */
err_start:
	if(rc)
		free(rc);

	/* Stop Video codec */
	snx_codec_stop(m2m);
err_init:
	/* un-initialize Video codec */

	snx_codec_uninit(m2m);
	if(m2m->m2m) {
		/* Stop ISP */
		snx_isp_stop(m2m);
		/* Un-initialize ISP */
		snx_isp_uninit(m2m);
	}
	/* Close video encode device */
	close(m2m->codec_fd);
	if(m2m->m2m) {
		/* Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);
	printf("============snx_m2m_cap_rc_flow============End\n");
	pthread_exit(0);
}

void snx_vc_data_stamp(int op, void *arg)
{

	struct snx_cds *cds = (struct snx_cds*)arg;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	unsigned int *font_table;
#endif
	if(strlen(cds->dev_name)==0) {
		printf("NO Devname for Data stamp\n");
		return;
	}

	switch (op)
	{
		case DS_SET_ALL:
			snx_cds_set_all(cds->dev_name, cds);
			break;
		case DS_GET_ALL:
			snx_cds_get_enable(cds->dev_name, (int *)&cds->enable);
			snx_cds_get_color(cds->dev_name, &cds->t_color, &cds->b_color);\
			snx_cds_get_color_attr(cds->dev_name, &cds->attr);
			snx_cds_get_position(cds->dev_name, &cds->pos, &cds->dim);
			//snx_cds_get_datastamp(cds->dev_name, cds->string, 0);
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			break;
		case DS_SET_EN:
			snx_cds_set_enable(cds->dev_name, cds->enable);
			break;
		case DS_GET_EN:
			snx_cds_get_enable(cds->dev_name, (int *)&cds->enable);
			break;
		case DS_SET_COLOR:
			snx_cds_set_color(cds->dev_name, &cds->t_color, &cds->b_color);
			break;
		case DS_GET_COLOR:
			snx_cds_get_color(cds->dev_name, &cds->t_color, &cds->b_color);
			break;
		case DS_SET_COLOR_ATTR:
			snx_cds_set_color_attr(cds->dev_name, &cds->attr);
			break;
		case DS_GET_COLOR_ATTR:
			snx_cds_get_color_attr(cds->dev_name, &cds->attr);
			break;
		case DS_SET_POS:
			snx_cds_set_position(cds->dev_name, &cds->pos, &cds->dim);
			break;
		case DS_GET_POS:
			snx_cds_get_position(cds->dev_name, &cds->pos, &cds->dim);
			break;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
#else
		case DS_SET_STRING:
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			snx_cds_set_string(cds->dev_name, cds->string, cds->scale);
			break;
#endif
		case DS_SET_FONT_STRING:
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
			if(cds->enable){
				//font_table = malloc(((cds->font_width <= 32?32:64)>>3)*(cds->font_height)*strlen(cds->string));
				font_table = snx_cds_get_font_table(cds);
				if(font_table)
			    	snx_cds_set_string(cds, NULL, font_table, 0);
				//free(font_table);
		    }
		    else
		        snx_cds_set_enable(cds->dev_name, 0);
#else
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			//snx_cds_set_font(cds, &ascii_2_font, &osd_font, sizeof(osd_font));
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA
			break;
		case DS_SET_DATA:
			snx_cds_set_datastamp(cds->dev_name, cds->string, strlen(cds->string));
			break;
		case DS_GET_DATA:
			snx_cds_get_datastamp(cds->dev_name, cds->string, 0);
			break;
		case DS_SET_BMP:
			snx_cds_get_scale(cds->dev_name, &cds->scale);
//			snx_cds_set_bmp(cds->dev_name, cds->string, cds->scale);
			snx_cds_set_bmp(cds);
			break;
		case DS_SET_SCALE:
			snx_cds_set_scale(cds->dev_name, cds->scale);
			break;
		case DS_GET_SCALE:
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			break;
	}


}
