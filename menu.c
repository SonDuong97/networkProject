#include <stdio.h>          /* These are the usual header files */

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