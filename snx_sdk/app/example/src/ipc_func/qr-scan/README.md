#qr-scan#
qr-scan is an example to scan the QRcode (quick response code) and decode the information

## How to use it ##

Execute "snx_scan", it would start to receive the image from the camera.
When it detects the correct qr-code image pattern, it starts to decode the embedded information.
The decoded content would be save in WIFI_TEMP_FILE, defined in snx_scan.c

## Android App ##
An example apk is in Android_app folder. You can type the content in the text field.
Click "OK" to generate QRcode image.


