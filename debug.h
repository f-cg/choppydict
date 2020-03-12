#define DEBUG 3 //DEBUG level

#if defined(DEBUG) && DEBUG > 0
 #define DEBUG_ERR(fmt, args...) fprintf(stderr, "DEBUG_ERR: %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_ERR(fmt, args...) /* Don't do anything in release builds */
#endif

#if defined(DEBUG) && DEBUG > 1
 #define DEBUG_WARN(fmt, args...) fprintf(stderr, "DEBUG_WARN: %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_WARN(fmt, args...)
#endif

#if defined(DEBUG) && DEBUG > 2
 #define DEBUG_INFO(fmt, args...) fprintf(stdout, "DEBUG_INFO: %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#else
 #define DEBUG_INFO(fmt, args...)
#endif

#define errorexit(fmt, args...) {fprintf(stderr, "errorexit: %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args);exit(1);}
