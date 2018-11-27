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

#define BACKLOG 20   /* Number of allowed connections */
#define BUFF_SIZE 1024

int parseMess(char *str, int *opcode, int *length, char *payload) {
    char temp_str[BUFF_SIZE];
    memcpy(temp_str, str, 1);
    temp_str[1] = '\0';
    *opcode = atoi(temp_str);

    memcpy(temp_str, str+1, 2);
    temp_str[2] = '\0';
    *length = atoi(temp_str);

    memcpy(temp_str, str+3, strlen(str) - 3);
    temp_str[strlen(str)-3] = '\0';
    strcpy(payload, temp_str);

    return 0;
}

// Processing the received message(str) and save to file
// Return opcode_type and key
int processData(char *str, char *ipv4Client)
{
    char payload[BUFF_SIZE];
    int opcode;
    int length;
    FILE *fp;
    parseMess(str, &opcode, &length, payload);

    switch(opcode) {
        case 0: // Save client's infomation
            if (length > 0) {
				char path[BUFF_SIZE] = "";
				strcat(path, "hardware-info/");
				strcat(path, ipv4Client);
				strcat(path, ".txt");
				if ((fp = fopen(path, "a")) == NULL) {
					printf("Can't save file client's infomation.\n");
					return 2;
				}
                fputs(payload, fp);
                fclose(fp);
            } else {
                return 1;
            }
            break;
    }
    return 0;
}


int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("The argument is error.\n");
		return 1;
	}

	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t	ret;
	fd_set	readfds, allset;
	char sendBuff[BUFF_SIZE], rcvBuff[BUFF_SIZE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
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
	
	//Step 4: Communicate with clients
	while (1) {
		readfds = allset;		/* structure assignment */
		nready = select(maxfd+1, &readfds, NULL, NULL, NULL);
		if(nready < 0){
			perror("\nError: ");
			return 0;
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

				if (--nready <= 0)
					continue;		/* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i]) < 0)
				continue;
			if (FD_ISSET(sockfd, &readfds)) {
				//receives message from client
				ret = recv(sockfd, rcvBuff, BUFF_SIZE, 0);
				if (ret <= 0){
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				} else {
					rcvBuff[ret] = '\0';
					int opcode, length;
					char payload[BUFF_SIZE];
					// parseMess(rcvBuff, &opcode, &length, payload);

					if (processData(rcvBuff, inet_ntoa(cliaddr.sin_addr)) == 1) {
						printf("Receiving client's infomation is successed.\n");
					}
					
					// printf("%d\n", ret);
					// strcpy(sendBuff, rcvBuff);
					// ret = send(sockfd, sendBuff, strlen(sendBuff), 0);
					// if (ret <= 0){
					// 	FD_CLR(sockfd, &allset);
					// 	close(sockfd);
					// 	client[i] = -1;
					// }
				}
			}

			if (--nready <= 0)
				break;		/* no more readable descriptors */
		}
	}
	
	return 0;
}
