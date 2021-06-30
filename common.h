#ifndef common_h
#define common_h

#include <malloc.h>
#include <string.h>
#include <arpa/inet.h> // for inet_aton
#include <unistd.h> // for usleep, fork
#include <stdlib.h> // for exit
#include <stdio.h>

#include "config.h"
#include "parse.h"
#include "cache.h"

//#define log_s(s) { if ( config.debug_level ) write(1,s,sizeof(s)); } 

void error(char *msg) { log_s(msg); perror(msg); exit(1); }
//#define error(msg) { log_s(msg); perror(msg); exit(1); }

//void error(char *msg);

void log_s(char *msg);
void log_b(char *prefix, void *ptr, int n);

void help();


#endif
