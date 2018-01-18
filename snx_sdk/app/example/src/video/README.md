## Video Examples ##
SN986 video fetch methods can be divided into two paths. One is M2M path, the other is Capture path.

M2M path is the main data path from ISP to Video endcoder.
We support up to 2 main pathes for 2 ISP channels. 

Capture path is way from Video encoder to Video encoder. So, it should depend on M2M path. 

* NOTE: 
 - One M2M can have zero or many Capture pathes with it, but one Capture path must have one m2m path with it.


#### M2M streams example ###
* For one stream -->  snx_m2m_one_stream.c
* For two m2m streams --> snx_m2m_two_stream.c
* snx_m2m_one_stream_with_rc.c
* snx_m2m_one_stream_yuv.c
* snx_m2m_dynamic_fps_bps.c

#### M2M input from File streams example ###
* snx_m2m_infile_stream_with_rc.c

#### M2M and Capture stream example ####
* snx_m2m_capture_2stream.c
* snx_m2m_capture_4stream.c
* snx_m2m_capture_2stream_with_rc.c

#### Data Stamp exmpale ####
* snx_vc_ds.c

#### Rate control example ####
* snx_m2m_one_stream_with_rc.c
* snx_m2m_dynamic_fps_bps.c
* snx_rc_ctl.c

#### Decode example ####
* snx_dec2videoout.c
* snx_dec2yuv420line.c

#### Transcode example ####
* snx_yuv2h264.c
  - YUV transcode to H264 bitstream or packing to AVI/MP4 file

#### EDR demo example ####
* sonix_edr_demo.c
  - On SN98605 demo board,You can press GPIO2 button to switch the 5 EDR functions as follows:
1.	Preview Front-Cam
2.	Preview Rear-Cam
3.	Preview PiP
4.	Playback Front-Cam
5.	Playback Rear-Cam

  - To build EDR demo firmware,Please do step by step as follows:
1.Build SN98605 firmware first as follows:
	cd buildscript
	make sn98605_360mhz_sf_defconfig
	make
	make install
	make example
2.Copy the uvc driver file to /lib/modules directory.	
	Copy st58600/driver/uvc/rootfs/lib/modules/2.6.35.12/kernel/drivers/snx_uvc.ko to 
	st58600/rootfs/lib/modules/2.6.35.12/kernel/drivers
3.Copy the necessary app binary files to /usr/bin directory.
	copy st58600/app/example/src/video/sonix_edr_demo to st58600/rootfs/usr/bin
	copy st58600/app/example/src/video/snx_m2m_one_stream_with_rc to st58600/rootfs/usr/bin
	copy st58600/app/example/src/video/snx_edr to st58600/rootfs/usr/bin
	copy st58600/driver/uvc/rootfs/driver-test-ap/SONiX_UVC_TestAP to st58600/rootfs/usr/bin
4.Disable other apps in the following start-up file.
	st58600/filesystem/rootfs/src/target/root/etc_default/init.d/rc.local    
	#echo "Start http-tunneling-serv ..."
	+#/bin/http-tunneling-serv &
	+#echo

	#echo "Start sonix-proj ..."
	+#/bin/sonix-proj &
	+#echo

	#echo "Start Two Way Audio ..."
	+#/bin/twowayaudio &

	#echo "Start user-configured modules ..."
	+#start_module
	+#echo
5.Enable EDR demo related drivers & app in the following start-up files.
	st58600/filesystem/rootfs/src/target/root/etc_default/init.d/rcS
	#For EDR demo
	+modprobe snx_vo snx_tv_lcd_sel=1
	+modprobe snx_uvc 
	+modprobe snx_vc_dec
	st58600/filesystem/rootfs/src/target/root/etc_default/init.d/rc.local    
	+sonix_edr_demo &
6.Re-build the firmware as following steps:
	cd buildscript
	make rootfs
	make rootfsimage
	make install

  
