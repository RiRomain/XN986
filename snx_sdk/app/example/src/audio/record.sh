#! /bin/sh
script_dir=`cd "$(dirname "$0")";pwd`

LD_LIBRARY_PATH=$script_dir/
if [ $# -lt 2 ]
then
	echo "Too few param" 
	echo "Usage: $0 format filename" 
	echo "    format : s8 / u8 / s16 / u16 / audio32 / audio32_16k / audio32_32k / opus / opus_48k / alaw"
	echo "             / mulaw / g726 / g722 / aac-lc_8k"
	echo ""
	exit 2
fi

echo "[RECORD]" 
gpio_ms1 -n 7 -m 1 -v 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
if [ $1 == "audio32" ]
then
$script_dir/snx_audio_record -d s16 -s 8 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
sleep 9
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_audio32 -m 0 -s 8 -a 16 -b 8 -i /tmp/audio.pcm -o $2
elif [ $1 == "audio32_16k" ]   
then
$script_dir/snx_audio_record -d s16 -s 16 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
sleep 9
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_audio32 -m 0 -s 16 -a 16 -b 16 -i /tmp/audio.pcm -o $2
elif [ $1 == "audio32_32k" ]   
then
$script_dir/snx_audio_record -d s16 -s 32 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
sleep 9
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_audio32 -m 0 -s 32 -a 16 -b 32 -i /tmp/audio.pcm -o $2

elif [ $1 == "opus" ]
then

$script_dir/snx_audio_record -d s16 -s 8 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
sleep 10
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_opus_encode -f 8000 16000 480 10 1 /tmp/audio.pcm $2

elif [ $1 == "opus_48k" ]
then

$script_dir/snx_audio_record -d s16 -s 48 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
sleep 10
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_opus_encode -f 48000 16000 2880 10 1 /tmp/audio.pcm $2

elif [ $1 == "aac-lc_8k" ]
then

$script_dir/snx_audio_record -d s16 -s 8 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
sleep 10
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_aac_encode -f 2 0 8000 1 64000 1 /tmp/audio.pcm $2

else
$script_dir/snx_audio_record -d $1 -f $2 &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19
sleep 9
fi

