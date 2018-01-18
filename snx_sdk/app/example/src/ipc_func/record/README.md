### About snx_record ###

snx_record fetch the video/audio stream and save them in the SD card.  

@  Support M2M/Capture path
@  Support /dev/video1 or /dev/video2
@  Programmable bitrate setting
@  Support video record triggered by motion detection alarm
@  Support AVI / MP4 file format

# How to use it

1. modified files/records.xml to set the resolution / fps / audio / package format.. etc.

# usage

Usage: ./snx_record [options]/n
Options:
        -h                 Print this message
        -m | --m2m         m2m path enable (default)
        -c | --capture          capture path enable (exclusive with m2m)
        -b | --bitrate          Video recording bitrate (Kbps) (Default 1024)
        -s | --schedule schedule recording enable (Dis: 0/ En: 1 Default is 1)
        -a | --alarm            alarm recording enable 
        -d | --device           Video recording device (1 / 2, Default 1)
        Ex:   ./snx_record -m -b 768


Reference: 


##############################
