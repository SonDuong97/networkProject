#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char const *argv[])
{
	char path[1024] = "image/son.txt";
	// char arr_str[4][1024] = {"infomation", "process_info", "mouse_operations", "keyboard_operations"};
	// for (int i = 0; i < 4; i++) {
	// 	printf("%s\n", arr_str[i]);
	// }

	// time_t t = time(NULL);
	// struct tm tm = *localtime(&t);
	// char datetime[1024];
	// strcat(datetime, tm.tm_year + 1900);
	// sprintf(datetime, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	// printf("%s\n", datetime);
// printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	if (rename("image/son.txt", "image/image.txt") == 0) {
		printf("doi ten thnah cong\n");
	} else printf("that bai\n");
	return 0;
}