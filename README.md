Description
The entire program was written in C. I ran it with a MacOS. gcc version 4.2.1

Inner Working

The program consists of two key files server.c and client.c. The way it works is we run server.c on port 4000 and run
client.c listening in on the same port and same ip address as the server. Since I ran both on the same machine the IP Address is: 127.0.0.1. Below is an example of how to run the program:

run buildServer.sh and runServer.sh in one terminal

and buildClient.sh and runClient.sh in another terminal

runServer/buildServer and buildClient/runClient contain the following commands:

gcc server.c -0 serv (compile server and create an executable called serv)
gcc client.c -0 cli (compile client and create an executable called cli)

./serv 4000 (run server on port 4000)

./cli 127.0.0.1 4000 (run client on the same IP as server and he same port number)

Then we can send messages from the client to the server and the server will immedietely reply with a response. For example, if the client sends the following message:

0x53545259aaaabbbbee0x0004

The server will respond with the following message:

0x535452594a4bee0x0000 --> since the request code was 4, it compressed the payload and set the status to OK (0x0000)

If the client sent the following message:

0x53545259AAAeebr0x0004

The server will respond with the following message:

<invalid: contains uppercase letters>

The way the program works is that it recieves the message and parses it into three components: header, payload, and request code. Depending on what the request code is (0x0001, 0x0002, 0x0003, or 0x0004) the server will give the appropriate response.If the RC is 0x0004, the payload is sent to the compress function. Here a new compressed payload is created that contains the number occurences of a letter followed by the letter. And once that is done successfully, the server sends back the header with the compressed payload and a status code set to OK (0x0000). The compress function takes into account the possibility of numbers or uppercase letters. If the RC isn't one of the 4 options, the respone consists of the header and status set to unsupported request type (0x0003).

If RC is 0x0002, the stats of the previous compression will be sent back to the client. It will send the bytesRecieved and the bytesSent along with the compression ratio.

If RC is 0x0003, the internal stats (bytesRecieved, bytes Sent, and compressionRatio is reset to zero)

If RC is 0x0001, the server sends back the header with the status code set to OK (0x0000)



Explanation of Libraries
***SERVER***

Socket():
int sockfd = socket(int domain, int type, int protocol)
1. domain --> communcation domain
	This specifies the address family. In our case AF_INET family which belongs to IPv4. 
2. type --> communcation type
	Our type is SOCK_STREAM which signifies that we will be using the TCP protocol.
	Other type is SOCK_DGRAM which signifies we are going to use the UDP protocol.

3. protocol --> communcation protocol
	This will be 0 because that is the default for the TCP protocol

When a socket is created with the socket function it exists in namespace so in address family but it has no address assigned to it.

Bind():
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)

1. sockfd --> returned by the socket function
2. Bind assigns the address specified by addr to the socket file descriptor sockfd
3. addrlen --> specifies the size in bytes of the address structure

The purpose of the sockaddr structure is to cast the structure pointer passed in addr to avoid compiler warnings. Additionally, it reutrns 0 on successful bind and -1 on failure.

listen():
int listen(int sockfd, int backlog)
Listen listens for the connection on the socket

1. sockfd --> file descriptor provided by the socket function
2. backlog --> number of connections a system can handle at a single time

Accept():
newsockfd = accept(sockfd, struct sockaddr *) & addr, &addrlen
returns a file descriptor called newsockfd. We use newsockfd for all communications after accept
Accept waits for the connection function from the client side before responding

***CLIENT***

connection():
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
similar to accept accept it returns 0 on success and -1 on failure


***SERVER & CLIENT***

int read(newsockfd, buffer, buffer_size)
2. buffer is the string we are going to pass
3. buffer size is the string size so message cannot be greater than the bugger size

int write(newsockfd, buffer, buffer_size)
2. buffer is the string we are going to pass
3. buffer size is the string size so message cannot be greater than the bugger size

close():
closes the connections



Assumptions:
I assumed that the client sends a message to the server in the following format:

header: 0x53545259
payload: aaaabbbee
request code: 0x0004  (compress)

header: 0x53545259
payload: 
request code: 0x0001  (ping)

header: 0x53545259
payload: 
request code: 0x0002  (get stats)

header: 0x53545259
payload: 
request code: 0x0003  (reset stats)

I'm assuming the requested payload can only consist of lowercase letters.















