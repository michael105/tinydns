#ifndef config_h
#define config_h

#include <stdlib.h>

#define CONFIG_TYPE_STRING 1
#define CONFIG_TYPE_INT    2
#define CONFIG_TYPE_RR     3

typedef struct TConfig
{
	char    *server_ip;
	uint16_t      server_port;
	char    *dns;
	int      dns_port;
	uint32_t cache_time;
	uint8_t  debug_level;
	char    *data;
} TConfig;

extern TConfig config;

void config_load();

#endif
