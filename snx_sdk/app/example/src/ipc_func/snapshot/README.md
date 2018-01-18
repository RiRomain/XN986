### About snx_snapshot ###

snx_snapshot is a snapshot service (JPEG) triggered by outside command. 

*  Support M2M / Capture Path
*  Support /dev/video1 or /dev/video2
*  Programable Width ,height, scale and fps. (depends on the M2M path)
*  Programable snapshot frame num.
*  Programable QP
*  YUV420 file format support
*  Y ONLY output support

# How to use it

1. touch /tmp/snapshot_en
2. /tmp/snapshot_list.txt shown when the capture is done.
3. The name of all captured pictures  would be save in snapshot_list.txt 

# usage

/media/mmcblk0p1 # ./snx_snapshot -h
Usage: ./snx_snapshot [options]/n
Version: V0.1.1
Options:
        -h                      Print this message
        -m                      m2m path enable (default is Capture Path)
        -o                      outputPath (default is /tmp)
        -i                      isp fps (Only in M2M path, default is 30)
        -f                      codec fps (default is 30 fps, NOT more than M2M path)
        -W                      Capture Width (Default is 1280, depends on M2M path)
        -H                      Capture Height (Default is 720, depends on M2M path)
        -q                      JPEG QP (Default is 60)
        -n                      Num of Frames to capture (Default is 3)
        -s                      scaling mode (default is 1,  1: 1, 2: 1/2, 4: 1/4 )
        -r                      YUV data output enable
        -v                      YUV capture rate divider (default is 5)
        M2M Example:   ./snx_snapshot -m -i 30 -f 30 -q 120 /dev/video1
        capture Example:   ./snx_snapshot -n 1 -q 120 /dev/video1

Reference: 


##############################
