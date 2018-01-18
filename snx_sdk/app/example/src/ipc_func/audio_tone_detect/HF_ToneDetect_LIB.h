
//extern "C" __declspec(dllimport) short ToneDetectCore(short *inputBuf, short InLength, long &CommandValT);
extern int ToneDetect(short *data, int offset, unsigned char *decode_buf);
extern int ToneInit(const char *SN);

#undef TD_DEBUG
