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

char time_wait[BUFF_SIZE];
// Make message from opcode, length, payload to send to server
char *makeMessage(int opcode, int length, char* payload)
{
    char* message = malloc(BUFF_SIZE+5);
    bzero(message, BUFF_SIZE+5);
    sprintf(message, "%d%04d", opcode, length);
    memcpy(message+5, payload, length);
    return message; 
}

int sendFile(int opcode, char* filename, int client_sock)
{
	int lSize, bytes_sent, byte_read;
    char buff[BUFF_SIZE];
    char *mess;
    int i;

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
        fprintf(stderr, "Failed to open file.\n");
        return -1;
    }
	// obtain file size:
	fseek (fp , 0 , SEEK_END);
	lSize = ftell (fp);
	rewind (fp);
	bzero(buff, BUFF_SIZE);
	while(lSize > 0) {
		if (lSize > BUFF_SIZE) {
			byte_read = fread (buff,1,BUFF_SIZE,fp);
  			if (byte_read != BUFF_SIZE) {fputs ("Reading error",stderr); exit (3);}
		} else {
			byte_read = fread (buff,1,lSize,fp);
			if (byte_read != lSize) {fputs ("Reading error",stderr); exit (3);}
		}
		// Sent message with opcode = 0: Send all infomation of client's computer
		mess = makeMessage(opcode, byte_read, buff);
		bytes_sent = send(client_sock, mess, BUFF_SIZE+5, 0);
		free(mess);
		if(bytes_sent <= 0){
			printf("Error: Connection closed.\n");
			return -1;
		}
		lSize -= byte_read;
		if (lSize <=0) {
			mess = makeMessage(opcode, 0, "");
			// Sent message with opcode = 0: Send all infomation of client's computer
			bytes_sent = send(client_sock, mess, 5, 0);
			free(mess);
			if(bytes_sent <= 0){
				printf("Error: Connection closed.\n");
				return -1;
			}
		}
	}
	fclose(fp);
	remove(filename);
	return 0;	
}

int sendAll(int client_sock)
{
	char command[BUFF_SIZE];
	sprintf(command,"xinput --test-xi2 --root > %s & sleep %s ; kill $!", TMP_OPERATION, time_wait);
    system(command);
    if (sendFile(2, TMP_OPERATION, client_sock) != 0) {
    	fprintf(stderr, "Sending mouse and keyboard operations is wrong.\n");
    	return -3;
    }
	sprintf(command,"lscpu > %s", TMP_INFO);
    system(command);
    if (sendFile(0, TMP_INFO, client_sock) != 0) {
    	fprintf(stderr, "Sending infomation is wrong.\n");
    	return -1;
    }

    sprintf(command,"top -b -n1 | head -n 10 > %s", TMP_PROCESSING);
    system(command);
    if (sendFile(1, TMP_PROCESSING, client_sock) != 0) {
    	fprintf(stderr, "Sending infomation is wrong.\n");
    	return -2;
    }
    sprintf(command,"import -window root %s", TMP_IMAGE);
    system(command);
    if (sendFile(3, TMP_IMAGE, client_sock) != 0) {
    	fprintf(stderr, "Sending image is wrong.\n");
    	return -4;
    }
    return 0;
}

int parseMess(char *str, int *opcode, int *length, char *payload) {
    char temp_str[5];
    memcpy(temp_str, str, 1);
    temp_str[1] = '\0';
    *opcode = atoi(temp_str);

    memcpy(temp_str, str+1, 4);
    temp_str[4] = '\0';
    *length = atoi(temp_str);

    memcpy(payload, str+5, *length);
    return 0;
}

int rcvTime(int client_sock)
{
	int bytes_received;
	int opcode, length;
	char payload[BUFF_SIZE];
	char buff[BUFF_SIZE+5];
	bytes_received = recv(client_sock, buff, BUFF_SIZE+5, MSG_DONTWAIT);
	if (bytes_received > 0) {
		parseMess(buff, &opcode, &length, payload);
		memcpy(time_wait, payload, length);
		time_wait[length] = 0;
	}
    return 0;
}



int main(int argc, char *argv[]){
	if (argc <= 2) {
		printf("The argument is error.\n");
		return 1;
	}
	char server_address[100] = "";
	strcpy(server_address, argv[1]);
	int server_port = atoi(argv[2]);
	int client_sock, i;
	char buff[BUFF_SIZE], sendBuff[BUFF_SIZE];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
	pthread_t tid;

	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_address);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
	// pthread_create(&tid, NULL, &mouse_and_keyboard, NULL);
	//Step 4: Communicate with server
	while (1) {	
		if (rcvTime(client_sock) != 0) {
			fprintf(stderr, "Receiving is wrong.\n");
		} else {
			// pthread_create(&tid, NULL, &mouse_and_keyboard, NULL);
		}


		if (sendAll(client_sock) != 0) {
			fprintf(stderr, "Sending is wrong.\n");
		}
	}
	
	// Step 4: Close socket
	close(client_sock);
	return 0;
}