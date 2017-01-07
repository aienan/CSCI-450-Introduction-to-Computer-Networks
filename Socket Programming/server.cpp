#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <signal.h>
#include "server.h"

Server::Server(string myServer){	// Server class constructor
	fill(this->matrix[0], this->matrix[0] + SERVER_NUM * SERVER_NUM, 0);	// Fill "0" into matrix
	this->myServer = myServer;
	this->myServerName = serverName();
	this->myServerIndex = serverIndex();
	this->serverChar[0] = 'A';
	this->serverChar[1] = 'B';
	this->serverChar[2] = 'C';
	this->serverChar[3] = 'D';
}
Server::~Server(){	// Server class destructor
	
}
string Server::serverName(){	// Get server's name (ex) serverA
	string fileName = SERVER_FILENAME;
	fileName += this->myServer;
	return fileName;
}
int Server::serverIndex(){	// Get server's index (ex) serverA for 0
	int serverIndex;
	if(this->myServerName.compare(SERVER_A) == 0){
		serverIndex = SERVER_A_INDEX;
	} else if(this->myServerName.compare(SERVER_B) == 0){
		serverIndex = SERVER_B_INDEX;
	} else if(this->myServerName.compare(SERVER_C) == 0){
		serverIndex = SERVER_C_INDEX;
	} else if(this->myServerName.compare(SERVER_D) == 0){
		serverIndex = SERVER_D_INDEX;
	}
	return serverIndex;
}
string Server::serverFileName(){	// Get server's file name (ex) serverA.txt
	string fileName = serverName();
	fileName += ".";
	fileName += SERVER_FILEEXT;
	return fileName;
}
void Server::fetchMine(){	// Fetch information from server's file name, and fill them into matrix
	string fileName_str = serverFileName();
	const char *fileName = fileName_str.c_str();
	ifstream inFile;
	stringstream buffer;
	string othServer;
	int cost;
	inFile.open(fileName);
	buffer << inFile.rdbuf();
	this->myInfo = buffer.str();	// Save file's string content
	inFile.seekg(0, inFile.beg);	// Set stream pointer to start
	cout << "The Server " + this->myServer + " has the following neighbor information:" << endl;
	cout << "Neighbor------Cost" << endl;
	while(inFile >> othServer >> cost){
		cout << othServer << "\t\t" << cost << endl;
		if(othServer.compare(SERVER_A) == 0){
			this->matrix[this->myServerIndex][SERVER_A_INDEX] = cost;
			this->matrix[SERVER_A_INDEX][this->myServerIndex] = cost;
		} else if(othServer.compare(SERVER_B) == 0){
			this->matrix[this->myServerIndex][SERVER_B_INDEX] = cost;
			this->matrix[SERVER_B_INDEX][this->myServerIndex] = cost;
		} else if(othServer.compare(SERVER_C) == 0){
			this->matrix[this->myServerIndex][SERVER_C_INDEX] = cost;
			this->matrix[SERVER_C_INDEX][this->myServerIndex] = cost;
		} else if(othServer.compare(SERVER_D) == 0){
			this->matrix[this->myServerIndex][SERVER_D_INDEX] = cost;
			this->matrix[SERVER_D_INDEX][this->myServerIndex] = cost;
		}
	}
	cout << endl;
}
string Server::getIP(struct addrinfo *addrInfo){	// Get IP address
	void *addr;
	char ipstr[INET6_ADDRSTRLEN];
	if(addrInfo->ai_family == AF_INET){	// IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)addrInfo->ai_addr;
		addr = &(ipv4->sin_addr);
	} else {	// IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addrInfo->ai_addr;
		addr = &(ipv6->sin6_addr);
	}
	inet_ntop(addrInfo->ai_family, addr, ipstr, sizeof ipstr);
	return string(ipstr);
}
unsigned short int Server::getPort(int sockfd){	// Get port number. From stackoverflow
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(sockfd, (struct sockaddr *)&sin, &len);
	return ntohs(sin.sin_port);
}
void *get_in_addr(struct sockaddr *sa){	// This code is from Beej's Guide to Network Programming
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void Server::tcpConnect(){	// Establish TCP connection with client. This code is from Beej's Guide to Network Programming
	int portClient = CLIENT_TCP_PORT;
	ostringstream ostr;
	ostr << portClient;
	string portClientStr = ostr.str();
	int sockfd;	// Socket File Descriptor
	int numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	string sendMsg;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(HOST_NAME, portClientStr.c_str(), &hints, &servinfo) != 0){
		perror("getaddrinfo");
		exit(1);
	}
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("socket");
			continue;
		}
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("connect");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		exit(1);
	}
	sendMsg = this->myServerName + string("\n") + this->myInfo;	// Attach server name and Neighbor information to the message
	if (send(sockfd, &sendMsg[0], strlen(sendMsg.c_str()), 0) == -1){	// Send neighbor information to the client
		perror("send");
	}
	cout << "The Server " << this->myServer << " finishes sending its neighbor information to the Client with TCP port number " << ((struct sockaddr_in *)(p->ai_addr))->sin_port << " and IP address " << getIP(servinfo) << endl;
	cout << "For this connection with the Client, the Server " << this->myServer << " has TCP port number " << getPort(sockfd) << " and IP address " << getIP(p) << endl;
	cout << endl;
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {	// Receive data
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';
	freeaddrinfo(servinfo); // all done with this structure
	close(sockfd);
}
void Server::udpServer(){	// Open UDP Server. This code is from Beej's Guide to Network Programming
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int status, port;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXDATASIZE];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	
	if(this->myServerIndex == SERVER_A_INDEX){
		port = SERVER_A_UDP_PORT;
	} else if(this->myServerIndex == SERVER_B_INDEX){
		port = SERVER_B_UDP_PORT;
	} else if(this->myServerIndex == SERVER_C_INDEX){
		port = SERVER_C_UDP_PORT;
	} else if(this->myServerIndex == SERVER_D_INDEX){
		port = SERVER_D_UDP_PORT;
	}
	ostringstream ostr;
	ostr << port;
	string portStr = ostr.str();
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	
	if ((status = getaddrinfo(HOST_NAME, portStr.c_str(), &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		exit(1);
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		exit(1);
	}
	freeaddrinfo(servinfo);
	
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	buf[numbytes] = '\0';
	stringToMatrix(buf);	// Put string information into matrix
	struct hostent *hostName = gethostbyname(HOST_NAME);	// Get host's name
	cout << "The server " << this->myServer << " has received the network topology from the Client with UDP port number " << getPort(sockfd) << " and IP address " << hostName->h_name << " as follows:" << endl;
	cout << endl;
	printEdgeCost();
	cout << endl;
	cout << "For this connection with Client, the Server " << this->myServer << " has UDP port " << getPort(sockfd) << " and IP address " << hostName->h_name << endl;
	cout << endl;
	close(sockfd);
}
void Server::stringToMatrix(char *buf){
	string str(buf);
	stringstream ss;
	int num;
	ss << str;
	for(int i=0; i < SERVER_NUM; i++){
		for(int j=0; j < SERVER_NUM; j++){
			ss >> num;
			this->matrix[i][j] = num;
		}
	}
}
void Server::printEdgeCost(){
	cout << "Edge------Cost" << endl;
	for(int i=0; i < SERVER_NUM; i++){
		for(int j=i+1; j < SERVER_NUM; j++){
			if(this->matrix[i][j] != 0){
				cout << serverChar[i] << serverChar[j] << "\t\t" << this->matrix[i][j] << endl;
			}
		}
	}
}
void Server::printMatrix(){
	for(int i=0; i < SERVER_NUM; i++){
		for(int j=0; j < SERVER_NUM; j++){
			cout << this->matrix[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}
void Server::bootUp(){	// Boot the server
	cout << "The Server " + this->myServer + " is up and running."<< endl;
	cout << endl;
	fetchMine();
	tcpConnect();
	udpServer();
}
void sigchld_handler(int s){	// This code is from Beej's Guide to Network Programming
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

int main(int argc, char* argv[]){	// main function
	string serverStr;
	pid_t pid;	// Child's process id
	int serverCount, status;
	struct sigaction sa;
	bool parent;
	
	parent = true;	// Set this process as parent
	serverCount = 0;	// Initially there are zero server
	while(parent && serverCount < SERVER_NUM){	// When this process is parent and number of servers are smaller than SERVER_NUM,
		sa.sa_handler = sigchld_handler; // reap all dead processes
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			perror("sigaction");
			exit(1);
		}
		if(serverCount == SERVER_A_INDEX){
			serverStr = "A";
		} else if(serverCount == SERVER_B_INDEX){
			serverStr = "B";
		} else if(serverCount == SERVER_C_INDEX){
			serverStr = "C";
		} else if(serverCount == SERVER_D_INDEX){
			serverStr = "D";
		}
		pid = fork();	// Make child process
		if(pid == 0){	// Child process
			parent = false;	// Set as child process
			Server *server = new Server(serverStr);	// Make server's instance
			server->bootUp();	// Boot up the server
		} else if (pid < 0) {	// When there is fork() error,
			perror("Error on fork()");
		} else {	// Parent process
			// pid = waitpid(pid, &status, 0);	// Wait for the child process
			serverCount++;	// Increase the number of servers
		}
	}
	// parent = true;	// Set this process as parent
	// serverCount = 0;	// Initially there are zero server
	// while(parent && serverCount < SERVER_NUM){
		// sa.sa_handler = sigchld_handler; // reap all dead processes
		// sigemptyset(&sa.sa_mask);
		// sa.sa_flags = SA_RESTART;
		// if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			// perror("sigaction");
			// exit(1);
		// }
		// if(serverCount == SERVER_A_INDEX){
			// serverStr = "A";
		// } else if(serverCount == SERVER_B_INDEX){
			// serverStr = "B";
		// } else if(serverCount == SERVER_C_INDEX){
			// serverStr = "C";
		// } else if(serverCount == SERVER_D_INDEX){
			// serverStr = "D";
		// }
		// pid = fork();	// Make child process
		// if(pid == 0){	// Child process
			// parent = false;	// Set as child process
			// Server *server = new Server(serverStr);	// Make server's instance
			// server->udpServer();	// Boot up the server
		// } else if (pid < 0) {	// When there is fork() error,
			// perror("Error on fork()");
		// } else {	// Parent process
			// // pid = waitpid(pid, &status, 0);	// Wait for the child process
			// serverCount++;	// Increase the number of servers
		// }
	// }
	
	return 0;
}
