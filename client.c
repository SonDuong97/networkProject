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
	int msg_len, bytes_sent, bytes_received;
	char sendBuff[BUFF_SIZE], rcvBuff[BUFF_SIZE];

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
	
	while (1) {
		printf("Enter the string: \n");
		fflush(stdin);
		fgets(sendBuff, BUFF_SIZE, stdin);
		msg_len = strlen(sendBuff) - 1;
		printf("%d\n", msg_len);
		// Sent the name of the file
		bytes_sent = send(client_sock, sendBuff, msg_len, 0);
		if(bytes_sent <= 0){
			printf("Error: Connection closed.\n");
			break;
		}

		// Receive responsing message from server
		bytes_received = recv(client_sock, rcvBuff, BUFF_SIZE, 0);
		if (bytes_received <= 0){
			printf("Error: Connection closed.\n");
			break;
		}
		rcvBuff[bytes_received] = '\0';
		printf("Reply from server: %s %lu\n", rcvBuff, strlen(rcvBuff));
		

		printf("----------------------------------------------------\n");
	}
	
	
	//Step 4: Close socket
	close(client_sock);
	return 0;
}