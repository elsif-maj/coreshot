#include <stdio.h>

void logging_log(char *msg) {
	printf("%s\n", msg);

	/*
	* 1) create formatted (timestamped) msg
	* 2) log to console
	* 3) write it to file
	*
	*/
}

void logging_logo() {
	printf("                                            _           _   \n");
	printf("                     ___ ___  _ __ ___  ___| |__   ___ | |_ \n");
	printf("                    / __/ _ \\| '__/ _ \\/ __| '_ \\ / _ \\| __|\n");
	printf("                   | (_| (_) | | |  __/\\__ \\ | | | (_) | |_ \n");
	printf("                    \\___\\___/|_|  \\___||___/_| |_|\\___/ \\__|\n");
	printf("                                                            \n");
}

void logging_server_start() {
	logging_logo();
}
