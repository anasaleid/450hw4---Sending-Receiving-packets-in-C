#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {	
	if(argc<4) {
		printf("Usage: ./sender <ip> <port> <filename>\n");
	}
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("Creating socket failed: ");
		exit(1);
	}
	
	struct sockaddr_in addr; 	// internet socket address data structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2])); // byte order is significant
	inet_pton(AF_INET,argv[1],&addr.sin_addr.s_addr);
	
	char buf[1400];
	memset(&buf,0,sizeof(buf));
	FILE *f=fopen(argv[3],"r");
	if(!f) {
		perror("problem opening file");
	}
	//get file size
	fseek(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	int numPackets = fileSize/1400;
	if(numPackets == 0){
		numPackets = 1;
	}
	int currPacket = 0;
	char test[255];
	int len = sizeof(addr);
	while(currPacket < numPackets) {

		if(fgets(buf,1500,f) == NULL){
			break;
		}
		char sbuf[1500];
		memset(&sbuf,0,sizeof(sbuf));
		sprintf(sbuf, "%d#%s", currPacket, buf);
		
		int send_count = sendto(sock, sbuf, strlen(sbuf), MSG_CONFIRM, (const struct sockaddr*)&addr, len);
		if(send_count<0) { perror("Send failed");	exit(1); }

		int recv_count = recvfrom(sock, test, sizeof test, MSG_WAITALL, (struct sockaddr*)&addr, &len);
		if(recv_count<0) { perror("Receive failed");	exit(1); }

		char num[5];
		sprintf(num, "%d", currPacket);
		printf("%s\n", test);
		if(strcmp(test, num) == 0){
			currPacket++;
		}
		else{
			printf("Got %s\n At: %d", test, currPacket);
		}
	}
	char fin[255] = "FIN";
	sendto(sock, fin, strlen(fin), MSG_CONFIRM, (const struct sockaddr*)&addr, len);
	shutdown(sock,SHUT_RDWR);
	close(sock);
}


