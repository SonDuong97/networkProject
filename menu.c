#include <stdio.h>          /* These are the usual header files */

int sendTimeWait() {
	int sockfd;
	printf("Enter time wait: ");
	while(1) {
		if (scanf("%d", &time) > 0) {
			time_wait = time;
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

int main() {
	int ret;
	int choose;
	which(1) {
		printf("1. Change time ().\n2. Search by IP\n3. Search by date\nChoose: ");
		scanf("%d", &choose);	
		switch(choose) {
			case 1: 
				sendTime();
				break;
			case 2:
				break;
			case 3:
				break;
		}
	}
	return 0;
}