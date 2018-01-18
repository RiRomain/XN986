### Video_Output ###

#### Video_Output Example ####
* Video width / height setting support
* Video Output window X / Y / Width / height setting support
* NTSC / PAL support

##### Usage #####
Usage: snx_videoout_test [options]
Options:
	-h Print this message...
	-R Frame Rate
	-W Frame Width
	-H Frame Height
	-x Output start x
	-y Output start y
	-n Output Width
	-v Output Height
	-o TV mode 0: ntsc 1: pal

##### Example #####
./snx_videoout_test -R 30 -W 640 -H 480 -x 0 -y 0 -n 640 -v 480 -o 0
