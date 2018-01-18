/**

 */

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define V4L2_CID_TRANS_TIME_MSEC        (V4L2_CID_USER_BASE + 0x1000)
#define V4L2_CID_TRANS_NUM_BUFS         (V4L2_CID_USER_BASE + 0x1001)
#define V4L2_CID_DATASTAMP_FONT_NUM     (V4L2_CID_USER_BASE + 0x1002)


#define perror_exit(cond, func)\
	if (cond) {\
		fprintf(stderr, "%s:%d: ", __func__, __LINE__);\
		perror(func);\
		exit(EXIT_FAILURE);\
	}

#define error_exit(cond, func)\
	if (cond) {\
		fprintf(stderr, "%s:%d: failed\n", func, __LINE__);\
		exit(EXIT_FAILURE);\
	}

#define perror_ret(cond, func)\
	if (cond) {\
		fprintf(stderr, "%s:%d: ", __func__, __LINE__);\
		perror(func);\
		return ret;\
	}

