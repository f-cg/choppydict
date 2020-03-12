#define NEXT(s) (s + strlen(s) + 1)
#define WL 32
#define STORAGE_SIZE 64 * 1024
#define CACHEDIR "/home/lily/.choppydict/"
#define words_file CACHEDIR "words_list"
#define cache_file CACHEDIR "words_cache"
#ifdef WGET
	#define GET "wget -q -O - "
#else
	#define GET "/usr/bin/curl --silent "
#endif
#define DICTCN "http://dict.cn/"
#define DICTCNSZ sizeof(DICTCN)
