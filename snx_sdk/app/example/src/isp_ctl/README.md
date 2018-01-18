### ISP Control ###

#### OSD SET ####

	./snx_isp_ctl -c 0 --osdset-en 1 --osdset-datastr sonix --osdset-ts 1 --osdset-template 123456789./-:sonix

#### Motion Detection SET ####

	./snx_isp_ctl -c 0 --mdset-en 1 --mdset-thre 200 --mdset-blkmask 0x00ff00ff 0x00ff00ff 0x00ff00ff 0xffffffff 0xffffffff 0xffffffff
	
	./snx_isp_ctl -c 0 --mdget-blkrepo

#### Mirror/Flip SET ####

* mirror
	./snx_isp_ctl -c 0 --mfset-mode 1 
  
* flip
	./snx_isp_ctl -c 0 --mfset-mode 2

* mirror&flip
	./snx_isp_ctl -c 0 --mfset-mode 3 

#### Private Mask SET ####

	./snx_isp_ctl -c 0 --pmset-en 1 --pmset-area 0xff00ff00 0xff00ff00 0xff00ff00 0x0 0x0 0x0

#### Contrast/Sharpness/Saturation/Hue/Brightness SET

	./snx_isp_ctl -c 0 --filterset-contrast 32 --filterset-sharp 3 --filterset-sat 64 --filterset-hue 180 --filterset-bright 64
