#ifndef H_ANALYZE_CACHE
#define H_ANALYZE_CACHE

// Your header code goes here, between #define and #endif

#define BILLION 1000000000L
#define CACHE_MAX_SIZE 16*1024*1024
#define CACHE_MAX_SIZE_EXT 2*1024*1024

char buffer[CACHE_MAX_SIZE];
char buffer_ext[CACHE_MAX_SIZE_EXT];

#endif

