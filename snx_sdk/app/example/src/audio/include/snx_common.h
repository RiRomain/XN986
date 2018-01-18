/**

 */

#define CLEAR(x) memset (&(x), 0, sizeof (x))



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

