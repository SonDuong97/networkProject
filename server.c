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
#include <pthread.h> 
#include <time.h>
#include <dirent.h> 
#include "cjson/cJSON.h"

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


fd_set	readfds, allset, writefds;
int nready, client[FD_SETSIZE], maxi, time_wait = 10;
typedef struct client_detail {
	cJSON *value;
	struct client_detail *next;
} Client_detail;

Client_detail *head = NULL;

int insertFirstPos(cJSON *client) {
	Client_detail *temp = (Client_detail *)malloc(sizeof(Client_detail));
	temp->value = client;
	temp->next = head;
	// Update first list address
	head = temp;
	return 0;
}

int getDataFromFile(char *ip, cJSON *client) {
	char path[BUFF_SIZE] = "";
	sprintf(path, "result/%s.txt", ip);
	FILE *fin = fopen(path, "r");
	char temp[BUFF_SIZE] = "";
	char json[BUFF_SIZE] = "";
	int curr = 0;
	cJSON *results = NULL;
	results = cJSON_AddArrayToObject(client, "results");
	if (cJSON_AddStringToObject(client, "ip", ip) == NULL)
    {
        printf("k them dc.\n");
    }
	if (fin == NULL) {
		return -1;
	}

	while (!feof(fin)){
		strcpy(temp, "");
		fgets(temp, BUFF_SIZE, fin);
		strcat(json, temp);
		if (strcmp(temp, "}\n") == 0) {
			json[strlen(json) - 1] = '\0';
			cJSON *result = cJSON_Parse(json);
			cJSON_AddItemToArray(results, result);
			curr++;
			strcpy(json, "");
		}
	}

	fclose(fin);
	return 0;
}

int searchByIp() {
	char ip[BUFF_SIZE];
	cJSON *res_client = cJSON_CreateObject();
	cJSON *results;
	cJSON *result;
	char command[BUFF_SIZE];
	int i = 1, select = 0, back;
	printf("Enter a ipv4: \n");
	scanf("%s", ip);
	if (getDataFromFile(ip, res_client) == -1) {
		printf("Ip khong ton tai.\n");
		return -1;
	}
	// char *str = cJSON_Print(res_client);
	// printf("%s\n", str);

	// ip = cJSON_GetObjectItemCaseSensitive(res_client, "ip");
	while (1) {
		i = 1;
		results = cJSON_GetObjectItemCaseSensitive(res_client, "results");
		cJSON_ArrayForEach(result, results) {
			cJSON *datetime = cJSON_GetObjectItemCaseSensitive(result, "datetime");
			printf("%d. %s\n", i, datetime->valuestring);
			i++;
		}
		back = i;
		printf("%d. BACK\n", back);
		printf("Enter a number: \n");
		while (1) {
			scanf("%d", &select);
			if ((select > 0) && (select <= back)) {
				break;
			} else {
				printf("Wrong number. Enter again: \n");
			}
		}
		if (select == back) {
			break;
		}
		i = 1;
		cJSON_ArrayForEach(result, results) {
			if (i != select) {
				i++;
				continue;
			}
			cJSON *infomation = cJSON_GetObjectItemCaseSensitive(result, "infomation");
			cJSON *keyboard_mouse_operations = cJSON_GetObjectItemCaseSensitive(result, "keyboard_mouse_operations");
			cJSON *process_info = cJSON_GetObjectItemCaseSensitive(result, "process_info");
			cJSON *image = cJSON_GetObjectItemCaseSensitive(result, "image");
			
			sprintf(command, "xdg-open %s", infomation->valuestring);
			system(command);
			sprintf(command, "xdg-open %s", process_info->valuestring);
			system(command);
			sprintf(command, "xdg-open %s", keyboard_mouse_operations->valuestring);
			system(command);
			sprintf(command, "xdg-open %s", image->valuestring);
			system(command);
			break;
		}
		system("clear");
	}
	
	return 0;
}

int searchByTime(char *time, cJSON *arr_res) {
	Client_detail *temp = head;
	cJSON *result = NULL;
	cJSON *results = NULL;
	cJSON *datetime = NULL;
	cJSON *ip = NULL;
	int flag = 0; // if flag = 0 then no result else had result
	cJSON *res = cJSON_AddArrayToObject(arr_res, "results");

	while (temp != NULL) {
		ip = cJSON_GetObjectItemCaseSensitive(temp->value, "ip");
		results = cJSON_GetObjectItemCaseSensitive(temp->value, "results");
		cJSON_ArrayForEach(result, results) {
			datetime = cJSON_GetObjectItemCaseSensitive(result, "datetime");
			if (strcmp(datetime->valuestring, time) == 0) {
				cJSON *buff = cJSON_CreateObject();
				cJSON *infomation = cJSON_GetObjectItemCaseSensitive(result, "infomation");
				cJSON *keyboard_mouse_operations = cJSON_GetObjectItemCaseSensitive(result, "keyboard_mouse_operations");
				cJSON *process_info = cJSON_GetObjectItemCaseSensitive(result, "process_info");
				cJSON *image = cJSON_GetObjectItemCaseSensitive(result, "image");
				cJSON_AddStringToObject(buff, "ip", ip->valuestring);
				cJSON_AddStringToObject(buff, "infomation", infomation->valuestring);
				cJSON_AddStringToObject(buff, "keyboard_mouse_operations", keyboard_mouse_operations->valuestring);
				cJSON_AddStringToObject(buff, "process_info", process_info->valuestring);
				cJSON_AddStringToObject(buff, "image", image->valuestring);
				cJSON_AddStringToObject(buff, "datetime", datetime->valuestring);
			    cJSON_AddItemToArray(res, buff);
			    flag = 1;
			}
		}
		temp = temp->next;
	}

	return flag > 0;
}

int searchTime() {
	char datetime[NAME_SIZE], command[NAME_SIZE];
	cJSON *arr_res = cJSON_CreateObject();
	cJSON *results;
	cJSON *result;
	int i = 1, select, back;
	char c;
	printf("Enter a datetime(yyyy-mm-dd hh:mm:ss): ");
	scanf("%c%[^\n]s", &c, datetime);

	if (searchByTime(datetime, arr_res) == 1) {
		while (1) {
			i = 1;
			results = cJSON_GetObjectItemCaseSensitive(arr_res, "results");
			cJSON_ArrayForEach(result, results) {
				cJSON *ip = cJSON_GetObjectItemCaseSensitive(result, "ip");
				printf("%d. %s\n", i, ip->valuestring);
				i++;
			}
			back = i;
			printf("%d. BACK\n", back);
			printf("Enter a number: \n");
			while (1) {
				scanf("%d", &select);
				if ((select > 0) && (select <= back)) {
					break;
				} else {
					printf("Wrong number. Enter again: \n");
				}
			}

			if (select == back) {
				break;
			}

			i = 1;
			cJSON_ArrayForEach(result, results) {
				if (i != select) {
					i++;
					continue;
				}
				cJSON *infomation = cJSON_GetObjectItemCaseSensitive(result, "infomation");
				cJSON *keyboard_mouse_operations = cJSON_GetObjectItemCaseSensitive(result, "keyboard_mouse_operations");
				cJSON *process_info = cJSON_GetObjectItemCaseSensitive(result, "process_info");
				cJSON *image = cJSON_GetObjectItemCaseSensitive(result, "image");

				sprintf(command, "xdg-open %s", infomation->valuestring);
				system(command);
				sprintf(command, "xdg-open %s", process_info->valuestring);
				system(command);
				sprintf(command, "xdg-open %s", keyboard_mouse_operations->valuestring);
				system(command);
				sprintf(command, "xdg-open %s", image->valuestring);
				system(command);
				break;
			}
			system("clear");
		}
	} else {
		printf("No result.\n");
	}

	return 0;
}

int init() {
	head = NULL;
	DIR *d;
	struct dirent *dir;
	d = opendir("result");
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if ((strcmp(dir->d_name, ".") != 0) && (strcmp(dir->d_name, "..") != 0)) {
				cJSON *client = cJSON_CreateObject();
				char ip[BUFF_SIZE] = "";
				strcpy(ip, dir->d_name);
				ip[strlen(ip) - 4] = '\0'; //get ip from filename

				getDataFromFile(ip, client);
				insertFirstPos(client);
				// printf("%s\n", ip);
			}
		}
		closedir(d);
	}
	return 0;
}

void freeList(Client_detail* head)
{
   Client_detail* tmp;

   while (head != NULL)
    {
       tmp = head;
       head = head->next;
       free(tmp);
    }

}

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
    fputs("\n", fp);
    free(out);
    fclose(fp);
    cli_info->json = cJSON_CreateObject();
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

// Processing the received message(str) and save to file
// Return opcode_type and key
int processData(ClientInfo* cli_info, char *str)
{
    char payload[BUFF_SIZE];
    int opcode;
    int length;
    FILE *fp;
    parseMess(str, &opcode, &length, payload);
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
char *makeMessage(int opcode, int length, char* payload)
{
    char* message = malloc(BUFF_SIZE+5);
    bzero(message, BUFF_SIZE+5);
    sprintf(message, "%d%04d", opcode, length);
    memcpy(message+5, payload, length);
    return message; 
}

int sendTime(int sockfd, int *time_wait) {
	char *mess;
	int ret;
	char timeWait[5];
	sprintf(timeWait, "%d", *time_wait);
	mess = makeMessage(4, strlen(timeWait), timeWait);
	ret = send(sockfd, mess, BUFF_SIZE + 5, 0);
	free(mess);
	return ret;
}

int sendTimeWait() {
	int sockfd;
	int ret, t, i;
	printf("Enter time wait: ");
	while(1) {
		if (scanf("%d", &t) > 0) {
			time_wait = t;
			break;
		} else printf("Re-enter: ");
	}

	for (i = 0; i <= maxi; i++) {
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
}

void *showMenu(void *arg) {
	int ret, choose, i;
	pthread_detach(pthread_self());
	while(1) {
		init();
		printf("1. Change time ().\n2. Search by IP\n3. Search by date\nChoose: ");
		scanf("%d", &choose);	
		switch(choose) {
			case 1: 
				sendTimeWait();
				break;
			case 2:
				searchByIp();
				break;
			case 3:
				searchTime();
				break;
		}
		freeList(head);
	}
}



int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("The argument is error.\n");
		return 1;
	}

	int i, maxfd, listenfd, connfd, sockfd, choose;

	ssize_t	ret;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	ClientInfo Client[FD_SETSIZE];
	char * mess, rcvBuff[BUFF_SIZE + 5];
	int port = atoi(argv[1]);
	pthread_t tid; 
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
	
	pthread_create(&tid, NULL, showMenu, client);
	//Step 4: Communicate with clients
	while (1) {
		readfds = allset;		/* structure assignment */
		writefds = allset;
		nready = select(maxfd+1, &readfds, &writefds, NULL, NULL);
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
				bzero(rcvBuff, BUFF_SIZE + 5);
				ret = recv(sockfd, rcvBuff, BUFF_SIZE + 5, MSG_WAITALL);
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
