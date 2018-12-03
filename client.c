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
#include "cjson/cJSON.h"

#define BUFF_SIZE 1024
#define TMP_FILENAME "tmp.txt"
#define TMP_IMAGE "image.png"

// Make message from opcode, length, payload to send to server
char *makeMessage(int opcode, int length, char* payload)
{
    char* message = malloc(BUFF_SIZE+5);
    bzero(message, BUFF_SIZE+5);
    sprintf(message, "%d%04d", opcode, length);
    memcpy(message+5, payload, length);
    return message; 
}

int getOutput(char *command, char *output_str) {
	FILE *fp = popen(command, "r");
	char buff[BUFF_SIZE];
	if (fp == NULL) {
		return 1;
	}

	while (!feof(fp)) {
		fgets(buff, BUFF_SIZE, fp);
		strcat(output_str, buff);
	}
	
	return 0;
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
		bytes_sent = send(client_sock, mess, byte_read+5, 0);
		if(bytes_sent <= 0){
			printf("Error: Connection closed.\n");
			return -1;
		}
		lSize -= byte_read;
		if (lSize <=0) {
			mess = makeMessage(opcode, 0, "");
			// Sent message with opcode = 0: Send all infomation of client's computer
			bytes_sent = send(client_sock, mess, 5, 0);
			if(bytes_sent <= 0){
				printf("Error: Connection closed.\n");
				return 0;
			}
		}
	}
	fclose(fp);
	remove(filename);
	
	return 1;	
}


int saveInfo()
{
    char str[32768], temp[32768];
    FILE *fp;

    getOutput("lshw -short | head -n 20", temp);
    strcat(str, temp);
    strcat(str, "FLAG_END_FIELD\n");
    strcpy(temp, "");
    getOutput("top -b -n1 | head -n 100", temp);
    strcat(str, temp);
    strcat(str, "FLAG_END_FIELD\n");
    system("import -window root image.png");

    if ((fp = fopen(TMP_FILENAME, "w")) == NULL) {
    	fprintf(stderr, "Failed to open file.\n");
        return -1;
    }
    fputs(str, fp);
    fclose(fp);
  
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
		
	//Step 4: Communicate with server
	
	while (1) {		
		saveInfo();
		if (sendFile(0,TMP_FILENAME, client_sock) <= 0) {
			fprintf(stderr, "Failed to open file.\n");
			goto FINISH;
		} else {
			printf("Xong filename\n");
		}
		if (sendFile(1,TMP_IMAGE, client_sock) <= 0) {
			fprintf(stderr, "Failed to open file.\n");
			goto FINISH;
		} else {
			printf("Xong image\n");
		}
		FINISH:
		printf("Delay:\n");
		fgets(sendBuff, 100, stdin);
	}
	
	// Step 4: Close socket
	close(client_sock);
	return 0;
}