#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <math.h>


char magic[10], payload[100], status[6], compressedPayload[200], response[200];		//components of the buffer stored as global variables
int bytesReceived = 0, bytesSent = 0, compressRatio = 0;
int isValid = 0, lenCompressedPayload = 0;																	//checks if the payload is valid (contains no numbers or uppercase letters)


void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void parser(char buffer[255])
{
		//clearing the payload of any previous values in memory
		memset(payload, 0, sizeof(payload));

		/*parsing the message into 3 components: header, payload, and status*/
		int i, k = 0;
		for (i = 0; i < 10; i++)
		{
			magic[k] = buffer[i];
			k++;
			//printf("Magic char: %c\n", magic[i]);		
		}

		printf("Magic: %s\n", magic);

		k = 0;
		char val;
		while(buffer[i] != '0')
		{
			payload[k] = buffer[i];
			i++;
			k++;
		}

		printf("payload is: %s\n", payload);

		int total = i + 6;
		k = 0;
		char val_two;
		for (i = i; i < total; i++)
		{
			status[k] = buffer[i];
			k++;
		}

		printf("status is: %s\n", status);


		/*determine if the buffer sent by server is valid*/
		for (i = 0; i < strlen(payload); i++)
		{
			//cannot contain numbers or upper case letter
			int val = payload[i];
			if ((val > 47) && (val < 58))
			{
				isValid = 1;
				break;
			}
			else if ((val > 64) && (val < 91))
			{
				
				isValid = 2;
				break;
			}
			else
			{
				isValid = 0;
			}

		}

}

int compress(char payload[], int len)
{
	int i = 1, count = 0, n = 0;
	char prev, current, countStr;

	//clearing the compressedPayload of any previous values in memory
	memset(compressedPayload, 0, sizeof(compressedPayload));

	while (i <= len)
	{
		prev = payload[i-1];
		current = payload[i];

		if (prev == current)
		{
			count++;
			i++;
		}
		else {
			if ((count != 0) && (count != 1)) {
				count++;
				countStr = count + '0';
				compressedPayload[n] = countStr;
				n++;
				compressedPayload[n] = prev;
				n++;
				count = 0;
				i++;
			}
			else if (count == 1) {
				compressedPayload[n] = prev;
				n++;
				compressedPayload[n] = prev;
				n++;
				i++;
				count = 0;
			}
			else {
				compressedPayload[n] = prev;
				n++;
				i++;
				count = 0;
			}
		}
	}

	compressedPayload[n+1] = '\0';

	//printf("compressedPayload: %s\n", compressedPayload);
	return (strlen(compressedPayload));
}

int main(int argc, char *argv[])
{
	if (argc < 2)																			//if port number isn't provided
	{	
		fprintf(stderr, "Port No not provided. Program terminated\n");
		exit(1);
	}


	int listenfd, connfd, portno, n;
	char buffer[255];																		//buffer will store messages

	struct sockaddr_in serv_addr, cli_addr;													//spckaddr_in provides us the internet addresses of the server and client
	socklen_t clilen;																		//32 bit data type

	listenfd = socket(AF_INET, SOCK_STREAM, 0);												//TCP
	if (listenfd < 0)																		//socket function has resulted in a failure
	{
		error("Error opening Socket.");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));											//clears any data it refers to
	portno = atoi(argv[1]);																	//argv[0] is the filename, argv[1] is the port number

	serv_addr.sin_family = AF_INET;					
	serv_addr.sin_addr.s_addr = INADDR_ANY;	
	serv_addr.sin_port = htons(portno);														//host to network shot

	if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("Binding Failed");															//return -1
	}

	listen(listenfd, 5);																	//can listen to up to 5 clients
	clilen = sizeof(cli_addr);

	while(1)
	{

		connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &clilen);

		if (connfd < 0)
		{
			error("Error on Accept");
		}

		/*now we will communicate between client and server*/

		bzero(buffer, 255);																	//clear anything that may be in our buffer
		n = read(connfd, buffer, 255);														//there will be a corresponding write function in the client
		if (n < 0)																			//if read fails
		{
			error("Error on reading");
		}

		parser(buffer);																		//parses the buffer into the header, payload, and status

		if (strncmp(status, "0x0001", 6) == 0)												//If RC is 1 (Ping Request)
		{
			//printf("status: %s\n", status);
			sprintf(response, "%s%s", "0x53545259", "0x0000");
		}
		else if (strncmp(status, "0x0002", 6) == 0)											//If RC is 2 (Get Stats Request)
		{
			//sprintf(response, "%s%s", toArray(bytesReceived), toArray(bytesSent));
			sprintf(response, "%d%d%d", bytesReceived, bytesSent, compressRatio);
		}
		else if (strncmp(status, "0x0003", 6) == 0)											//If RC is 3 (Reset Stats Request)
		{
			bytesReceived = 0;
			bytesSent = 0;
			compressRatio = 0;
			lenCompressedPayload = 0;
			sprintf(response, "%s%s", "0x53545259", "0x0000");
		}
		else if (strncmp(status, "0x0004", 6) == 0) 	 									//If RC is 4 (Compress Requestuest)
		{
			compress(payload, strlen(payload));

			bytesReceived = strlen("0x53545259") + strlen(payload);
			int lenPayload = strlen(payload);
			printf("payload length: %d\n", lenPayload);

			lenCompressedPayload = compress(payload, strlen(payload));
			printf("compressed payload length: %d\n", lenCompressedPayload);

			bytesSent = strlen("0x53545259") + lenCompressedPayload;
			compressRatio = bytesSent/bytesReceived;


			if (isValid == 0)
				sprintf(response, "%s%s%s", "0x53545259", compressedPayload, "0x0000");
			else if (isValid == 1) 															//if the there are numbers in the payload
				sprintf(response, "<invalid: contains numbers>");
			else if (isValid == 2) 															//if the there are uppercase letters in the payload
				sprintf(response, "<invalid: contains uppercase letters>");

			printf("New PayLoad %s\n", response);

		}
		else
		{
			sprintf(response, "%s%s", "0x53545259", "0x0003");								//response consists of header with status code set to unsported request type
		}

		printf("Request: %s\n", buffer);													//printing message that client gives
		bzero(buffer, 255);																	//clear the buffer so server can reply with a message
	


		n = write(connfd, response, strlen(response));										//corresponding function read in client
		//n = write(connfd, buffer, strlen(buffer));	//corresponding function read in client

		memset(response, 0, sizeof(response));

		if (n < 0)																			//if write fails
		{
			error("Error on writing");
		}
		close(connfd);

	}

	close(listenfd);
	return 0;

}