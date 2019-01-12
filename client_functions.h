#ifndef _CLIENT_FUNCTIONS_H_
#define _CLIENT_FUNCTIONS_H_

#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <libgen.h>
#include <pthread.h>
#include "cjson/cJSON.h"

#define BUFF_SIZE 1024
#define TMP_FILENAME "tmp.txt"
#define TMP_INFO "info.txt"
#define TMP_IMAGE "image.png"
#define TMP_OPERATION "event.txt"
#define TMP_PROCESSING "log.txt"
#define MAX_LENGTH 256

int sendFile(int opcode, char* filename, int client_sock);
int sendAll(int client_sock);
int rcvTime(int client_sock);

#endif