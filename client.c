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

#define BUFF_SIZE 1024

// Make message from opcode, length, payload to send to server
char *makeMessage(int opcode, int lenght, char* payload)
{
    char* message = malloc(BUFF_SIZE+4);
    bzero(message, BUFF_SIZE+3);
    sprintf(message, "%d%02d%s", opcode, lenght, payload);
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

int main(int argc, char *argv[]){
	if (argc <= 2) {
		printf("The argument is error.\n");
		return 1;
	}
	char server_address[100] = "";
	strcpy(server_address, argv[1]);
	int server_port = atoi(argv[2]);
	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received, i;
	char sendBuff[BUFF_SIZE], rcvBuff[BUFF_SIZE];
	char data[BUFF_SIZE] = "";

	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_address);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! \n");
		return 0;
	}

	//Sent client's infomation
	if (getOutput("lscpu", data) == 1) {
		printf("Error.\n");
	} else {
		msg_len = strlen(data);
		i = 0;
		while(1) {
			char temp[100];
			char *mess;
			if (msg_len >= 99) {
				memcpy(temp, data + i, 99);
				temp[99] = '\0';
				mess = makeMessage(0, 99, temp);
			} else {
				memcpy(temp, data + i, msg_len);
				temp[msg_len] = '\0';
				mess = makeMessage(0, msg_len, temp);
			}

			// Sent message with opcode = 0: Send all infomation of client's computer
			bytes_sent = send(client_sock, mess, strlen(mess), 0);
			if(bytes_sent <= 0){
				printf("Error: Connection closed.\n");
				break;
			}

			i += 99;
			msg_len -= 99;
			if (msg_len <= 0) {
				mess = makeMessage(0, 0, "");
				// Sent message with opcode = 0: Send all infomation of client's computer
				bytes_sent = send(client_sock, mess, strlen(mess), 0);
				if(bytes_sent <= 0){
					printf("Error: Connection closed.\n");
					break;
				}
				break;
			}
		}
	}
	
	// Send information in 15s
	// while(1) {

	// }
	//Step 4: Close socket
	close(client_sock);
	return 0;
}