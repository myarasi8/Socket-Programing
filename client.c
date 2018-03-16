/*
filename server_ipaddress protno

argv[0] -> filename	argv[1] -> server_ipaddress	argv[2] -> portno

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>													//need this for read write and close
#include <netdb.h>													//allows us to use hostent structure

void error(const char *msg)
{
	perror(msg);
	exit(1);
}


int main(int argc, char *argv[])
{
	int sockfd, portno, n, i, inc;
	
	struct sockaddr_in serv_addr;					//sockaddr_in provides us the internet addresses of the server
	struct hostent *server;								//stores info about a given host such as hostname and internet portocol version 4 address

	char buffer[255];									//buffer will store messages

	if (argc < 3)
	{
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
	}

	portno = atoi(argv[2]);
	server = gethostbyname(argv[1]);					//gets ip address of server
	if (server == NULL)
	{
		fprintf(stderr, "Error, no such host");
	}


	while(1)
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);			//TCP
		if (sockfd < 0)										//socket function has resulted in a failure
		{
			error("Error opening Socket.");
			exit(1);
		}


		bzero((char*) &serv_addr, sizeof(serv_addr));		//clears any data it refers to
		serv_addr.sin_family = AF_INET;						
		bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);		//copies n bytes from hosten server to serv_addr
		serv_addr.sin_port = htons(portno);						//specifies host to network shot


		if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		{
			error("Connection Failed\n");
		}


		bzero(buffer, 255);								//clearing anything pointing to the buffer
		fgets(buffer, 255, stdin);						//pass our string to the server
		//fputs(buffer, stdout);
		printf("Sending: %s", buffer);

		n = write(sockfd, buffer, strlen(buffer));

		if (n < 0)										//if write fails
		{
			error("Error on writing\n");
		}

		bzero(buffer, 255);								//clear the buffer so server can reply with a message
	
		//fgets(buffer, 255, stdin);					//stdio.h lib, reads bytes from stream


		n = read(sockfd, buffer, 255);					//corresponding function read in client
		if (n < 0)										//if read fails
		{
			error("Error on reading\n");
		}

		printf("Reply: %s\n\n", buffer);

		close(sockfd);
	}

	return 0;
}


