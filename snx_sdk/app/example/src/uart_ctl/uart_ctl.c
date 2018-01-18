/**********************************************************************
 *
 * Function:    SPI to GPIO for IR-Cut/LED control program    
 *
 * Usage: 
 * 1. ./snx_uart_ctl  : will read from uart1
 * 2. ./snx_uart_ctl helloworld : will write "helloworld" to uart1
 **********************************************************************/
 #include <sys/types.h>
      #include <sys/stat.h>
      #include <fcntl.h>
      #include <termios.h>
      #include <stdio.h>
        
      #define BAUDRATE B115200
      #define MODEMDEVICE "/dev/ttyS1"
      #define _POSIX_SOURCE 1 /* POSIX compliant source */
      #define FALSE 0
      #define TRUE 1
        
      volatile int STOP=FALSE; 
       
	  int main(int argc,char *argv[])
      {
        int fd,c, res;
        struct termios oldtio,newtio;
        char buf[255];
        
        fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY  |  O_NONBLOCK |O_NDELAY);
        if (fd <0) {perror(MODEMDEVICE); exit(-1); }
        
        tcgetattr(fd,&oldtio); /* save current port settings */
        
        bzero(&newtio, sizeof(newtio));

        memcpy(&newtio, &oldtio, sizeof(struct termios));

        newtio.c_cflag &= ~PARENB;
        newtio.c_cflag &= ~CSTOPB;
        newtio.c_cflag &= ~CSIZE;
        newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        newtio.c_cflag &= ~CRTSCTS;

        newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

        newtio.c_oflag &= ~OPOST;
        
        /* set input mode (non-canonical, no echo,...) */
        newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
         
        newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
        newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */
        
        tcflush(fd, TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
        
		if (argc == 1) {        
	        while (STOP==FALSE) {       /* loop for input */
			  printf(".\n");usleep(100000);
	          res = read(fd,buf,255);   /* returns after 1 chars have been input */
	          if( res > 0) {
		          buf[res]=0;               /* so we can printf... */
		          printf(":%s:%d\n", buf, res);
		          if (buf[0]=='z') STOP=TRUE;
		      }
	        }
	    } else if (argc == 2) {
	    	int cnt = 0;
	    	res = 0;
	    	strcpy(buf, argv[1]);
	    	printf("Start to xmt : %s (%d)\n", buf,strlen(argv[1]));
	    	while (cnt < strlen(argv[1])) {

	    		res = write(fd, &buf[cnt], strlen(argv[1]) - cnt);
	    		if (res > 0) {
	    			printf("tx: %d\n",res);
	    			cnt += res;
	    		} else
	    			printf("write error (%d)\n", res);
	    	}
	    		
	    } else {

	    	printf("Wrong param\n");
	    }
        //tcsetattr(fd,TCSANOW,&oldtio);
        close(fd);
      }
