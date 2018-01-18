### About audio tone detection ###

Audio tone detection is used to receive the encrypted message via audio, mostly used in SSID/PASSWORD transmittion from mobile phone


# usage
./TDMain

   Detected Message would be saved in /tmp/ssid.txt

# NOTE: 
   Please make sure you have a audio device name, "snx_audio_pcm_48k_ex",  48K sample rate / S16LE 

# Dependence 
middleware_openssl, middleware_snx-ez, middleware_audio

# Received Message format:
--------------------------------------------------------------------------------------------------------------------
| Signature(4)Signature(4) |  Version(2) | SSID length(2) | SSID(0~32) | PW length(2) | PW(0~64) BID(4) | Mode(1)   |
--------------------------------------------------------------------------------------------------------------------

@ All displayed by ASCII

Example:
         Signature:3562
         Version:54
         SSID:TP-LINK-IPCAM
         PW:sonix123
         BID:F32A
         MODE: 4
         Result : 35625413TP-LINK-IPCAM08sonix123F32A4

         Signature(4B) : a-z, A-Z. 0-9
         Version(2B) : a-z, A-Z. 0-9
         SSID/PW Length(2B) : 0-9
         SSID : 0~32 bytes
         PW: 0 ~ 64 bytes
         BID(5B) : 0-9

@ Mode  Number
         Open  1
         WEP   2
         WPA   3
         WPA2  4

Magic number：
前面４位數(3562)為signature, 後面２位數(54)為版本號. 可用字元為a-z, A-Z. 0-9


##############################
