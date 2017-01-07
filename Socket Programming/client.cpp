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
#include <sstream>
#include <signal.h>
#include "client.h"

Client::Client(){
	fill(this->matrix[0], this->matrix[0] + SERVER_NUM * SERVER_NUM, 0);	// Fill "0" into matrix
	this->serverChar[0] = 'A';
	this->serverChar[1] = 'B';
	this->serverChar[2] = 'C';
	this->serverChar[3] = 'D';
}
Client::~Client(){
	
}
string Client::getIP(struct addrinfo *addrInfo){	// Get IP address
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
unsigned short int Client::getPort(int sockfd){	// Get port number. From stackoverflow
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(sockfd, (struct sockaddr *)&sin, &len);
	return ntohs(sin.sin_port);
}
void Client::dnsWork(){	// This code is from Beej's Guide to Network Programming
	int status;
	struct addrinfo hints, *servinfo;	// will point to the results
	int port = CLIENT_TCP_PORT;
	ostringstream ostr;
	ostr << port;
	string portStr = ostr.str();
	
	memset(&hints, 0, sizeof hints);	// Make sure the struct is empty
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if((status = getaddrinfo(HOST_NAME, portStr.c_str(), &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	this->clientAddrInfo = servinfo;
}
void sigchld_handler(int s){	// This code is from Beej's Guide to Network Programming
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}
void *get_in_addr(struct sockaddr *sa){	// This code is from Beej's Guide to Network Programming
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
string Client::matrixString(){	// Convert matrix into stringstream
	stringstream ss;
	for(int i=0; i < SERVER_NUM; i++){
		for(int j=0; j < SERVER_NUM; j++){
			ss << this->matrix[i][j] << " ";
		}
	}
	return ss.str();
}
void Client::printEdgeCost(){
	cout << "Edge------Cost" << endl;
	for(int i=0; i < SERVER_NUM; i++){
		for(int j=i+1; j < SERVER_NUM; j++){
			if(this->matrix[i][j] != 0){
				cout << serverChar[i] << serverChar[j] << "\t\t" << this->matrix[i][j] << endl;
			}
		}
	}
}
void Client::tcpServer(){	// This code is from Beej's Guide to Network Programming
	int sockfd, new_fd;	// Socket File Descriptor
	struct addrinfo *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char ipstr[INET6_ADDRSTRLEN];
	int numbytes, serverCount = 0;
	char buf[MAXDATASIZE];
	stringstream ss;
	string senderName, othServer;
	int cost, status, serverIndex, ssStart;
	int pipefd[2];
	pid_t cpid;
	
	// loop through all the results and bind to the first we can
	for(p = clientAddrInfo; p != NULL; p = p->ai_next){
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	cout << "The Client has TCP port number " << getPort(sockfd) << " and IP address " << getIP(this->clientAddrInfo) << endl;
	// cout << "The Client has TCP port number " << ((struct sockaddr_in *)(p->ai_addr))->sin_port << " and IP address " << getIP(this->clientAddrInfo) << endl;
	cout << endl;
	if(listen(sockfd, SERVER_NUM) == -1){
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	while(1){ // main accept() loop
		sin_size = sizeof their_addr;
		if((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
			if(errno == EINTR){
				continue;
			} else {
				perror("accept");
				exit(1);
			}
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), ipstr, sizeof ipstr);
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {	// Receive data
			perror("recv");
			exit(1);
		}
		buf[numbytes] = '\0';
		ss.clear();
		ss.str("");
		ss.str(buf);
		ss >> senderName;	// Get server name
		ssStart = ss.tellg();	// Start position of neighbor information
		ssStart += 1;	// Erase \n
		if(senderName.compare(SERVER_A) == 0){
			serverIndex = SERVER_A_INDEX;
		} else if(senderName.compare(SERVER_B) == 0){
			serverIndex = SERVER_B_INDEX;
		} else if(senderName.compare(SERVER_C) == 0){
			serverIndex = SERVER_C_INDEX;
		} else if(senderName.compare(SERVER_D) == 0){
			serverIndex = SERVER_D_INDEX;
		}
		while(ss >> othServer >> cost){	// Merge servers' data into one adjacency matrix
			if(othServer.compare(SERVER_A) == 0){
				this->matrix[serverIndex][SERVER_A_INDEX] = cost;
				this->matrix[SERVER_A_INDEX][serverIndex] = cost;
			} else if(othServer.compare(SERVER_B) == 0){
				this->matrix[serverIndex][SERVER_B_INDEX] = cost;
				this->matrix[SERVER_B_INDEX][serverIndex] = cost;
			} else if(othServer.compare(SERVER_C) == 0){
				this->matrix[serverIndex][SERVER_C_INDEX] = cost;
				this->matrix[SERVER_C_INDEX][serverIndex] = cost;
			} else if(othServer.compare(SERVER_D) == 0){
				this->matrix[serverIndex][SERVER_D_INDEX] = cost;
				this->matrix[SERVER_D_INDEX][serverIndex] = cost;
			}
		}
		cout << "The Client receives neighbor information from the Server " << this->serverChar[serverIndex] << " with TCP port number " << ((struct sockaddr_in *)&their_addr)->sin_port << " and IP address " << ipstr << endl;
		cout << endl;
		cout << "The Server " << this->serverChar[serverIndex] << " has the following neighbor information:" << endl;
		cout << "Neighbor------Cost" << endl;
		ss.clear();
		ss.seekg(ssStart, ios::beg);	// Set start of neighbor information
		cout << ss.rdbuf() << endl;
		cout << endl;
		cout << "For this connection with Server " << this->serverChar[serverIndex] << ", the Client has TCP port number " << getPort(new_fd) << " and IP address " << getIP(this->clientAddrInfo) << endl;
		cout << endl;
		close(new_fd);
		serverCount++;
		if(serverCount == SERVER_NUM){
			break;
		}
	}
}
void Client::udpConnect(){	// This code is from Beej's Guide to Network Programming
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int status, port;
	int numbytes, serverCount;
	ostringstream ostr;
	string portStr;
	
	serverCount = 0;
	while(serverCount < SERVER_NUM){
		if(serverCount == SERVER_A_INDEX){
			port = SERVER_A_UDP_PORT;
		} else if(serverCount == SERVER_B_INDEX){
			port = SERVER_B_UDP_PORT;
		} else if(serverCount == SERVER_C_INDEX){
			port = SERVER_C_UDP_PORT;
		} else if(serverCount == SERVER_D_INDEX){
			port = SERVER_D_UDP_PORT;
		}
		ostr.clear();
		ostr.str("");
		ostr << port;
		portStr = ostr.str();
		
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;
		
		if ((status = getaddrinfo(HOST_NAME, portStr.c_str(), &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
			exit(1);
		}
		// loop through all the results and make a socket
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				perror("talker: socket");
			continue;
			}
			break;
		}
		if (p == NULL) {
			fprintf(stderr, "failed to create socket\n");
			exit(1);
		}
		string message = matrixString();
		char *msg = &message[0];
		if ((numbytes = sendto(sockfd, msg, strlen(msg), 0, p->ai_addr, p->ai_addrlen)) == -1) {
			perror("sendto");
			exit(1);
		}
		struct hostent *hostName = gethostbyname(HOST_NAME);	// Get host's name
		if(hostName == NULL){
			perror("gethostbyname");
			exit(1);
		}
		cout << "The Client has sent the network topology to the network topology to the Server " << this->serverChar[serverCount] << " with UDP port number " << portStr.c_str() << " and IP address " << hostName->h_name << " as follows:" << endl;
		cout << endl;
		printEdgeCost();
		cout << endl;
		cout << "For this connection with Server " << this->serverChar[serverCount] << ", the Client has UDP port number " << getPort(sockfd) << " and IP address " << hostName->h_name << endl;
		cout << endl;
		serverCount++;
	}
	freeaddrinfo(servinfo);
	close(sockfd);
}
void Client::minimumSpanningTree(){	// Calculate minimum spanning tree
	int minVal = -1, curVal;
	int edgeNum = SERVER_NUM * (SERVER_NUM - 1) / 2;
	Node edgeArr[edgeNum];
	int selectNum = SERVER_NUM - 1;
	int selectArr[selectNum];
	Node finalArr[selectNum];
	int count = 0;
	bool isCostZero;
	bool checkTreeArr[SERVER_NUM];
	bool isTree;
	for(int i=0; i < SERVER_NUM; i++){	// Add matrix information into Node class array
		for(int j=i+1; j < SERVER_NUM; j++){
			edgeArr[count].set(i, j, this->matrix[i][j]);
			count++;
		}
	}
	for(int i=0; i < edgeNum + 1 - selectNum; i++){
		selectArr[0] = i;
		for(int j=i+1; j < edgeNum + 2 - selectNum; j++){
			selectArr[1] = j;
			for(int k=j+1; k < edgeNum + 3 - selectNum; k++){
				selectArr[2] = k;
				curVal = 0;
				isCostZero = false;
				for(int l=0; l < SERVER_NUM; l++){	// checkTreeArr initialization
					checkTreeArr[l] = false;
				}
				isTree = true;
				for(int l=0; l < selectNum; l++){
					if(edgeArr[selectArr[l]].cost == 0){
						isCostZero = true;
						break;
					}
					checkTreeArr[edgeArr[selectArr[l]].row] = true;
					checkTreeArr[edgeArr[selectArr[l]].col] = true;
					curVal += edgeArr[selectArr[l]].cost;
				}
				for(int l=0; l < SERVER_NUM; l++){
					if(checkTreeArr[l] == false){
						isTree = false;
						break;
					}
				}
				if((minVal == -1 || curVal < minVal) && !isCostZero && isTree){
					minVal = curVal;
					for(int l=0; l < selectNum; l++){
						finalArr[l] = edgeArr[selectArr[l]];
					}
				}
			}
		}
	}
	cout << "The Client has calculated a tree. The tree cost is " << minVal << endl;
	cout << endl;
	cout << "Edge------Cost" << endl;
	for(int i=0; i < selectNum; i++){
		cout << serverChar[finalArr[i].row] << serverChar[finalArr[i].col] << "\t\t" << finalArr[i].cost << endl;
	}
	cout << endl;
}
void Client::bootUp(){
	dnsWork();
	tcpServer();
	udpConnect();
	minimumSpanningTree();
}

int main(int argc, char* argv[]){
	Client *client = new Client();
	client->bootUp();
	
	return 0;
}
