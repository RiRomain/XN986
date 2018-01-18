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

echo "[PLAY]" 
gpio_ms1 -n 7 -m 1 -v 1
$script_dir/snx_audio_vol_ctl -d spk -v 19
if [ $1 == "audio32" ]
then
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_audio32 -m 1 -g 2.5 -s 8 -b 8 -a 16 -i $2 -o /tmp/audio.pcm
$script_dir/snx_audio_playback -d s16 -s 8 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19

elif [ $1 == "audio32_16k" ]
then
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_audio32 -m 1 -g 2.5  -s 16 -b 16 -a 16 -i $2 -o /tmp/audio.pcm
$script_dir/snx_audio_playback -d s16 -s 16 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19

elif [ $1 == "audio32_32k" ]
then
LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_audio32 -m 1 -g 2.5  -s 32 -b 32 -a 16 -i $2 -o /tmp/audio.pcm
$script_dir/snx_audio_playback -d s16 -s 32 -f /tmp/audio.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19

elif [ $1 == "opus" ]
then

LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_opus_decode -f 8000 $2 /tmp/audio_oput.pcm
$script_dir/snx_audio_playback -d s16 -s 8 -f /tmp/audio_oput.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d mic -v 19

elif [ $1 == "opus_48k" ]
then

LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_opus_decode -f 48000 $2 /tmp/audio_oput.pcm
$script_dir/snx_audio_playback -d s16 -s 48 -f /tmp/audio_oput.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d spk -v 19

elif [ $1 == "aac-lc_8k" ]
then

LD_LIBRARY_PATH=$script_dir/ $script_dir/snx_aac_decode -f $2 /tmp/audio_oput.pcm
$script_dir/snx_audio_playback -d s16 -s 8 -f /tmp/audio_oput.pcm &
sleep 1
$script_dir/snx_audio_vol_ctl -d spk -v 19

else
$script_dir/snx_audio_playback -d $1 -f $2 &
sleep 1
$script_dir/snx_audio_vol_ctl -d spk -v 19
fi

