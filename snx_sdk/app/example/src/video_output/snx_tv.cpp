

#include "tv.h"
#include "capture.h"
#include "output.h"
#include "codec.h"

#ifdef USE_M_TABLE
#include <syslog.h>
extern "C"
{
	#include "snx_m_table.h"	
}
#endif 
static struct TV_OUT tv_out;

CTvViewer::CTvViewer(CVideoCapture *capture,CVideoCodec *codec, CVideoOutput *output):
					_capture(capture),
					_codec(codec),
					_output(output)

{
	return;
}

CTvViewer::~CTvViewer()
{
	return;
}


int CTvViewer::Open(void)
{
	return 0;
}

int CTvViewer::Run(void)
{
	Start();
	return 0;
}

int CTvViewer::Stop(void)
{
	Kill();
	return 0;
}

int CTvViewer::Close(void)
{
	return 0;
}

void *CTvViewer::Thread(void)
{
	struct snx_frame_ctx b;
	int nbuffers = 0, capbuffers = 0, outbuffers = 0;
  
	if(_output->GetMemType() == V4L2_MEMORY_USERPTR){
		if(_capture)
			capbuffers = _capture->GetBufferCount();
		if(_codec)
			capbuffers = _codec->_capture->GetBufferCount();
		outbuffers = _output->GetBufferCount();
		nbuffers = (capbuffers<outbuffers)?(capbuffers):(outbuffers);		
	}
	
	JThread::ThreadStarted();
	while(IsRunning()){
		if(_output->GetMemType() == V4L2_MEMORY_MMAP){
			snx_frame_ctx_t cb = NULL, ob = NULL;
			ob = _output->DeFrame();
			if(!ob){
				break;
			}
			if(_capture)
				cb = _capture->DeFrame();
			if(_codec)
				cb = _codec->_capture->DeFrame();
			if(!cb){
				break;
			}

			memcpy(ob->userptr, cb->userptr, ob->size);
			_output->EnFrame(ob);
	
			if(_capture)
				_capture->EnFrame(cb);
			if(_codec)
				_codec->_capture->EnFrame(cb);
		}
		
		if(_output->GetMemType() == V4L2_MEMORY_USERPTR){
			snx_frame_ctx_t cb = NULL, ob = NULL;
			if(_capture)
				cb = _capture->DeFrame();
			if(_codec)
				cb = _codec->_capture->DeFrame();
			if(!cb){
				fprintf(stderr, "capture buf null\n");
				break;
			}
			b.index = cb->index%nbuffers;
			b.userptr = cb->userptr;
			b.length = cb->length;
			_output->EnFrame(&b);
			ob = _output->DeFrame();
			if(!ob){
				fprintf(stderr, "output buf null\n");
				break;
			}
			if(_capture)
				_capture->EnFrame(cb);
			if(_codec)
				_codec->_capture->EnFrame(cb);
		}
	}
	return NULL;
}


static void sigterm(int sig)
{
	if(tv_out.tv){
		tv_out.tv->Stop();
		tv_out.tv->Close();
		tv_out.tv= NULL;
	}
	if(tv_out.output){
		tv_out.output->Stop();
		tv_out.output->Close();
		tv_out.output= NULL;
	}
	if(tv_out.capture){
		tv_out.capture->Stop();
		tv_out.capture->Close();
		tv_out.capture= NULL;
	}
	if(tv_out.codec){
		tv_out.codec->_capture->Stop();
		tv_out.codec->_capture->Cleanup();
		tv_out.codec->Close();
		tv_out.codec= NULL;
	}	
#ifdef USE_M_TABLE
	if (close_max_res_video((char*)FMT_RAW, NULL) <0)
	{
		syslog(LOG_ERR, "close_max_res_video failed\n");
	}
#endif
	printf("signal terminate\n");
	exit(0);
}


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message...\n"  
        "-R Frame Rate\n"
        "-W Frame Width\n"
        "-H Frame Height\n"
        "-x Output start x\n"
        "-y Output start y\n"
        "-n Output Width\n"
        "-v Output Height\n"
        "-o TV mode 0: ntsc 1: pal\n"
        "", argv[0]);   
}   

static const char short_options[] = "hmuR:W:H:p:x:y:n:v:o:";   
static const struct option long_options[] =
{
	     { "help", no_argument, NULL, 'h' },
	     { "rate", required_argument, NULL, 'R' },
       { "width", required_argument, NULL, 'W' }, 
       { "height", required_argument, NULL, 'H' }, 
       { "x", required_argument, NULL, 'x' },
       { "y", required_argument, NULL, 'y' },
       { "n", required_argument, NULL, 'n' },
       { "v", required_argument, NULL, 'v' },    
       { "o", required_argument, NULL, 'o' },      
       { 0, 0, 0, 0 }
};   

int main(int argc, char **argv)
{
	int width = 640, height = 480, rate = 10;
	int x = 0, y = 0, output_width = 640, output_height = 480;
	int nbuffers = 4;
  int tv_mode = 0;
	enum v4l2_memory io = V4L2_MEMORY_USERPTR;

	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   
		if (-1 == c)   
			break;   
		switch (c)
		{   
			case 0: /* getopt_long() flag */   
				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);   
			case 'u':   
				io = V4L2_MEMORY_USERPTR;   
				break;   
			case 'R':
				sscanf(optarg, "%d", &rate);
				break;
			case 'W':
				sscanf(optarg, "%d", &width);
				break;
			case 'H':
				sscanf(optarg, "%d", &height);
				break;
			case 'x':
				sscanf(optarg, "%d", &x);
				break;
			case 'y':
				sscanf(optarg, "%d", &y);
				break;
			case 'n':
				sscanf(optarg, "%d", &output_width);
				break;	
			case 'v':
				sscanf(optarg, "%d", &output_height);
				break;
      case 'o':
				sscanf(optarg, "%d", &tv_mode);
				break;  	
			default:   
				usage(stderr, argc, argv);   
				exit(EXIT_FAILURE);   
		}   
	}
  /* set tv mode 0:ntdc 2:pal */   
  if(tv_mode == 0)
	{
		system("echo 0x0 >/proc/vo/iface/tv/mode ");
	}
	else
	{
		system("echo 0x2 >/proc/vo/iface/tv/mode ");
	}

	signal(SIGINT , sigterm); /* Interrupt (ANSI).	  */	
	signal(SIGTERM, sigterm); /* Termination (ANSI).  */

	memset(&tv_out, 0x0, sizeof(tv_out));
	//first try isp device
  /* if want to run from isp, must only open one stream 
    in stream server or no run stream server process /bin/sonix_proj */
	fprintf(stderr, "try isp capture..\n");
	tv_out.capture= new CVideoCapture(width, height, rate, V4L2_MEMORY_MMAP, nbuffers);
	if(tv_out.capture->Open() < 0)
		goto vc;

	tv_out.capture->Run();
	goto start;
vc:
	fprintf(stderr, "try codec capture\n");
	tv_out.codec= new CVideoCodec();
	if(tv_out.codec->Open() < 0){
		fprintf(stderr, "no source data device for tv\n");
		exit(0);
	}
	tv_out.codec->_capture->Initialize(&width, &height, rate, V4L2_PIX_FMT_SNX420,
									V4L2_BUF_TYPE_VIDEO_CAPTURE, 
									V4L2_MEMORY_MMAP, nbuffers);

	tv_out.codec->_capture->Run();
start:
	tv_out.output = new CVideoOutput(VO_CH_0, x, y, width, height, output_width, output_height, io, nbuffers);
	tv_out.output->Open();
	tv_out.output->Run();
	if(tv_out.capture)
		tv_out.tv = new CTvViewer(tv_out.capture, NULL, tv_out.output);
	if(tv_out.codec)
		tv_out.tv = new CTvViewer(NULL, tv_out.codec, tv_out.output);
	tv_out.tv->Open();
	tv_out.tv->Run();
	
	while(1){
		sleep(1);
	}
	return 0;
}


