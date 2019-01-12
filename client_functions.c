#include "client_functions.h"

int sendFile(int opcode, char* filename, int client_sock)
{
	int lSize, bytes_sent, byte_read;
    int buff[MAX_LENGTH];
    int *mess;
    int i, len;

	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
        fprintf(stderr, "Failed to open file.\n");
        return -1;
    }
	// obtain file size:
	fseek (fp , 0 , SEEK_END);
	lSize = ftell (fp);
	rewind (fp);
	bzero(buff, MAX_LENGTH);
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
		len = (MAX_LENGTH + 2)* 4;
		bytes_sent = send(client_sock, mess, len, 0);
		free(mess);
		if(bytes_sent <= 0){
			printf("Error: Connection closed.\n");
			return -1;
		}
		lSize -= byte_read;
		if (lSize <=0) {
			mess = makeMessage(opcode, 0, buff);
			// Sent message with opcode = 0: Send all infomation of client's computer
			bytes_sent = send(client_sock, mess, len, 0);
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
	sprintf(command,"xinput --test-xi2 --root > %s & sleep %d ; kill $!", TMP_OPERATION, time_wait);
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

int rcvTime(int client_sock)
{
	int bytes_received;
	int opcode, length;
	int payload[MAX_LENGTH];
	int buff[MAX_LENGTH+2];
	int len = (MAX_LENGTH + 2) * 4;
	bytes_received = recv(client_sock, buff, len, MSG_DONTWAIT);
	if (bytes_received > 0) {
		parseMess(buff, &opcode, &length, payload);
		memcpy(&time_wait, payload, length);
	}
    return 0;
}