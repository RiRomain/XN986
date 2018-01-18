## record.sh ##
example
./record.sh format file
foramt = s8/u8/s16/u16/audio32/audio32_16k/audio32_32k/opus/opus_48k/alaw/mulaw/g726/g722/aac-lc_8k
file = encode filename

## play.sh ##
example
./play.sh format file
foramt = s8/u8/s16/u16/audio32/audio32_16k/audio32_32k/opus/opus_48k/alaw/mulaw/g726/g722/aac-lc_8k
file = decode filename

### For multi thread audio record ###
Please refer to snx_audio_record_2stream.c

### Volume control ###

MIC and Speaker Digital Gain --> snx_audio_vol_ctl

        "snx_audio_vol_ctl [options]"
         "-h Print this message"
         "-d | --device    mic or spk"
         "-m | --mute      mute this device"
         "-v | --vol       Set vol value (0 ~ 19)"
         "-i | --info      get the vol info"

MIC Analog Gain  -->snx_audio_sigma_vol_ctrl.c

		 "snx_audio_sigma_vol_ctrl [options]:"
         "-h Print this message"
         "-r Rough adjusting sound"
         "-f Fine adjusting sound"
         "-v Set vol value"
