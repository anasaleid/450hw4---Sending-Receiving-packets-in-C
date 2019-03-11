
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <signal.h>

void handle_alarm(int sig) {
	fprintf(stderr,"More than a second passed after the last packet. Exiting.\n");
	exit(1);
}

int main(int argc, char** argv) {

	if(argc<2) {
		printf("Usage: ./receiver <port>\n");
		exit(1);
	}
	signal(SIGALRM,handle_alarm);

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock < 0) {
		perror("Creating socket failed: ");
		exit(1);
	}

	struct sockaddr_in addr; 	// internet socket address data structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1])); // byte order is significant
	addr.sin_addr.s_addr = INADDR_ANY;

	struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));

	int res = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	if(res < 0) {
		perror("Error binding: ");
		exit(1);
	}

	char buf[1500];
	memset(&buf,0,sizeof(buf));
	int len;
	while(1) {
		len = sizeof(addr);
		int recv_count = recvfrom(sock, buf, 1500, MSG_WAITALL, (struct sockaddr *) &clientAddr, &len);
		
		if(strcmp(buf, "FIN") == 0){
			printf("Got FIN\n");
			break;
		}
		char *buf2 = strtok(buf, "#");
		//printf("%s\n", buf2);
		// stop receiving after a second has passed
		//alarm(1);

		if(recv_count<0) { perror("Receive failed");	exit(1); }
		write(1,buf,recv_count);

		int send_count = sendto(sock, buf2, strlen(buf2), MSG_CONFIRM, (const struct sockaddr *)&clientAddr, len);
		if(send_count<0) { perror("Send failed");	exit(1); }
	}

	shutdown(sock,SHUT_RDWR);
}


