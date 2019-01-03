#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "cjson/cJSON.h"
#include <time.h>

#define BACKLOG 20   /* Number of allowed connections */
#define BUFF_SIZE 1024
#define NAME_SIZE 256
#define MAX_LENGTH 256

#define FOLDER_INFO "info"
#define FOLDER_PROCESSING "processing"
#define FOLDER_RESULT "result"
#define FOLDER_IMG "images"
#define FOLDER_LOG "logs"
#define WAIT 0
#define FINISH 1
// #define TMP_INFO "info.txt"
// #define TMP_IMAGE "image.png"
// #define TMP_OPERATION "event.txt"
#define KEY_INFO "infomation"
#define KEY_PROCESSING "process_info"
#define KEY_KBMS "keyboard_mouse_operations"
#define KEY_IMAGE "image"

typedef struct client_info {
	int status;
	char ip_address[BUFF_SIZE];
	char result[BUFF_SIZE];
	char tmp_info[BUFF_SIZE];
	char tmp_image[BUFF_SIZE];
	char tmp_operation[BUFF_SIZE];
	char tmp_processing[BUFF_SIZE];
	cJSON *json;
} ClientInfo;

int setDatetime(ClientInfo* cli_info) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char datetime[2024];

	sprintf(datetime, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	// Set current date_time in obj Json
    if (cJSON_AddStringToObject(cli_info->json, "datetime", datetime) == NULL)
    {
        return -1;
    }

    return 0;
}

int setPath(ClientInfo* cli_info, char *key,char* extension, char* folder, char* path) {
	char path_file[2024];

	sprintf(path_file, "%s/%s[%ld].%s", folder , cli_info->ip_address, (unsigned long)time(NULL), extension);

    if ((rename(path, path_file)) != 0) {
		fprintf(stderr, "Can't rename image file.\n");
		return -1;
	}

    if (cJSON_AddStringToObject(cli_info->json, key, path_file) == NULL)
    {
        return -1;
    }

    return 0;
}

int saveJsonToFile(ClientInfo* cli_info) {
	char *out;
	FILE *fp;
	out = cJSON_Print(cli_info->json);
    if (out == NULL) {
        fprintf(stderr, "Failed to print computer.\n");
        return 1;
    }

    cJSON_Delete(cli_info->json);
	fp = fopen(cli_info->result,"a+");
    if (fp == NULL) {
    	fprintf(stderr, "Failed to open file.\n");
        return 1;
    }
    fputs(out, fp);
    free(out);
    fclose(fp);
    cli_info->json = cJSON_CreateObject();
    return 0;
}



int parseMess(int *mess, int *opcode, int *length, int *payload) {
    *opcode = mess[0];
    *length = mess[1];
    memcpy(payload, mess+2, *length);
    return 0;
}

// Processing the received message(str) and save to file
// Return opcode_type and key
int processData(ClientInfo* cli_info, int *str)
{
    int payload[MAX_LENGTH];
    int opcode;
    int length;
    FILE *fp;
    parseMess(str, &opcode, &length, payload);
    fprintf(stderr, "%d %d\n",opcode, length);
    switch(cli_info->status) {
    	case WAIT:
    		switch(opcode) {
        		case 0:
        			if (length != 0) {
        				if ((fp = fopen(cli_info->tmp_info, "a+")) == NULL) {
							printf("Can't open file client's infomation.\n");
							return 1;
						}
						fwrite(payload, 1, length, fp);
			            fclose(fp);
        			} else {
        				if (setPath(cli_info, KEY_INFO, "txt", FOLDER_INFO, cli_info->tmp_info) != 0) {
        					fprintf(stderr, "Setting info is wrong.\n");
        					return 1;
        				}
        			}
		            break;

		        case 1:
        			if (length != 0) {
        				if ((fp = fopen(cli_info->tmp_processing, "a+")) == NULL) {
							printf("Can't open file client's infomation.\n");
							return 1;
						}
						fwrite(payload, 1, length, fp);
			            fclose(fp);
        			} else {
        				if (setPath(cli_info, KEY_PROCESSING, "txt", FOLDER_PROCESSING, cli_info->tmp_processing) != 0) {
        					fprintf(stderr, "Setting info is wrong.\n");
        					return 1;
        				}
        			}

		            break;

		        case 2:
        			if (length != 0) {
		        		if ((fp = fopen(cli_info->tmp_operation, "a+")) == NULL) {
		        			printf("Can't open file client's mouse and keyboard event\n");
							return 1;
		        		}
		        		fwrite(payload, 1, length, fp);
		        		fclose(fp);
		        	} else {
		        		if (setPath(cli_info, KEY_KBMS, "txt", FOLDER_LOG, cli_info->tmp_operation) != 0) {
        					fprintf(stderr, "Setting info is wrong.\n");
        					return 1;
        				}
		        	}
		        	break;

		        case 3:
		        	if (length != 0) {
		        		if ((fp = fopen(cli_info->tmp_image, "ab+")) == NULL) {
		        			printf("Can't open file client's image.\n");
							return 1;
		        		}
		        		fwrite(payload, 1, length, fp);
		        		fclose(fp);
		        	} else {
		        		if (setPath(cli_info, KEY_IMAGE, "png", FOLDER_IMG, cli_info->tmp_image) != 0) {
        					fprintf(stderr, "Setting info is wrong.\n");
        					return 1;
        				}

        				if (setDatetime(cli_info) != 0) {
        					fprintf(stderr, "Can't set datetime.\n");
		        			return 1;
        				}

		        		if (saveJsonToFile(cli_info) != 0) {
		        			fprintf(stderr, "Can't save json.\n");
		        			return 1;
		        		}
		        	}
		        	break;
	        }
    		break;
    	case FINISH:
    		break;
    }
    return 0;
}

// Make message from opcode, length, payload to send to server
int *makeMessage(int opcode, int length, int* payload)
{
    int* message = malloc(BUFF_SIZE+2);
    bzero(message, BUFF_SIZE+2);
    // sprintf(message, "%d%04d", opcode, length);
    message[0] = opcode;
    message[1] = length;
    memcpy(message+2, payload, length);
    return message; 
}

int sendTime(int sockfd, int *time_wait) {
	int *mess;
	int ret;
	int len = (MAX_LENGTH + 2)*4;
	mess = makeMessage(4, 4, time_wait);
	ret = send(sockfd, mess, len, 0);
	free(mess);
	if (ret <= 0){
		return -1;
	}
	return 1;
}

int resole(int menuno, int choose) {
	switch (menuno) {
		case 0:
			
			break;
		case 1:
			break;
	}
}


int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("The argument is error.\n");
		return 1;
	}

	int i, maxi, maxfd, listenfd, connfd, sockfd, choose, time, menuno = 0, len;
	int nready, client[FD_SETSIZE];
	ssize_t	ret;
	fd_set	readfds, allset, writefds;
	int sendBuff[MAX_LENGTH+2];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	ClientInfo Client[FD_SETSIZE];
	int * mess, time_wait = 10, rcvBuff[MAX_LENGTH+2];
	int port = atoi(argv[1]);
	//Step 1: Construct a TCP socket to listen connection request
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}

	//Step 2: Bind address to socket
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	} 

	//Step 3: Listen request from client
	if(listen(listenfd, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}

	maxfd = listenfd;			/* initialize */
	maxi = -1;				/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	FD_SET(STDIN_FILENO, &allset);
	FD_SET(STDOUT_FILENO, &allset);
	
	//Step 4: Communicate with clients
	while (1) {
		readfds = allset;		/* structure assignment */
		writefds = allset;
		nready = select(maxfd+1, &readfds, &writefds, NULL, NULL);
		if(nready < 0){
			perror("\nError: ");
			return 0;
		}
		if (FD_ISSET(STDIN_FILENO, &readfds)) {
			scanf("%d", &time_wait);
			for (i = 0; i <= maxi; i++) {	/* check all clients for data */
				if ( (sockfd = client[i]) < 0)
					continue;
				if (FD_ISSET(sockfd, &allset)) {
					ret = sendTime(sockfd, &time_wait);
					if (ret <= 0){
						FD_CLR(sockfd, &allset);
						close(sockfd);
						client[i] = -1;
					}
				}
				if (--nready <= 0)
					break;		/* no more readable descriptors */
			}			
		} else {
			menuno = 100;
		}
		
		if (FD_ISSET(listenfd, &readfds)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			if((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen)) < 0)
				perror("\nError: ");
			else{
				printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr)); /* prints client's IP */
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] < 0) {
						client[i] = connfd;	/* save descriptor */
						break;
					}
				if (i == FD_SETSIZE){
					printf("\nToo many clients");
					close(connfd);
				}

				FD_SET(connfd, &allset);	/* add new descriptor to set */
				if (connfd > maxfd)
					maxfd = connfd;		/* for select */
				if (i > maxi)
					maxi = i;		/* max index in client[] array */

				Client[i].status = 0;
				strcpy(Client[i].ip_address, inet_ntoa(cliaddr.sin_addr));
				sprintf(Client[i].result,"%s/%s.txt",FOLDER_RESULT, inet_ntoa(cliaddr.sin_addr));
				sprintf(Client[i].tmp_info,"%s/%s.txt",FOLDER_INFO, inet_ntoa(cliaddr.sin_addr));
				sprintf(Client[i].tmp_image,"%s/%s.png",FOLDER_IMG, inet_ntoa(cliaddr.sin_addr));
				sprintf(Client[i].tmp_operation,"%s/%s.txt",FOLDER_LOG, inet_ntoa(cliaddr.sin_addr));
				sprintf(Client[i].tmp_processing,"%s/%s.txt",FOLDER_PROCESSING, inet_ntoa(cliaddr.sin_addr));
				Client[i].json = cJSON_CreateObject();
				ret = sendTime(connfd, &time_wait);
				if (ret <= 0){
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				}
				if (--nready <= 0)
					continue;		/* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &readfds)) {
				//receives message from client
				bzero(rcvBuff, MAX_LENGTH+2);
				len = (MAX_LENGTH + 2) * 4;
				ret = recv(sockfd, rcvBuff, len, 0);
				if (ret <= 0){
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				} else {
					if (processData(&Client[i], rcvBuff) == 0) {
					}
				}
			}
			if (--nready <= 0)
				break;		/* no more readable descriptors */
		}
	}
	
	return 0;
}
