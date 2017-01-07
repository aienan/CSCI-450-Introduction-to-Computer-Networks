#ifndef _SERVER_H_
#define _SERVER_H_

// #define HOST_NAME "nunki.usc.edu"
#define HOST_NAME "localhost"
#define SERVER_NUM 4
#define USC_ID 738
#define SERVER_A_UDP_PORT (21000+USC_ID)
#define SERVER_B_UDP_PORT (22000+USC_ID)
#define SERVER_C_UDP_PORT (23000+USC_ID)
#define SERVER_D_UDP_PORT (24000+USC_ID)
#define CLIENT_TCP_PORT (25000+USC_ID)
#define SERVER_FILENAME "server"
#define SERVER_FILEEXT "txt"
#define SERVER_A "serverA"
#define SERVER_B "serverB"
#define SERVER_C "serverC"
#define SERVER_D "serverD"
#define SERVER_A_INDEX 0
#define SERVER_B_INDEX 1
#define SERVER_C_INDEX 2
#define SERVER_D_INDEX 3
#define MAXDATASIZE 1024

using namespace std;

class Server {
	int matrix[SERVER_NUM][SERVER_NUM];	// Adjacency matrix
	string myServer;	// Server's parameter such as A, B, C or D
	string myServerName;	// Server's name such as serverA, serverB, serverC or serverD
	int myServerIndex;	// Server's index for adjacency matrix
	char serverChar[SERVER_NUM];	// Server's character such as A, B, C or D
	string myInfo;	// Neighbor information in the file
	string myIP;	// My IP address
	
	string serverName();	// Get server's name (ex) serverA
	int serverIndex();	// Get server's index (ex) 0 for serverA
	string serverFileName();	// Get server's file name (ex) serverA.txt
	void fetchMine();	// Fetch information from server's file name, and fill them into matrix
	string getIP(struct addrinfo *addrInfo);	// Get IP address
	unsigned short int getPort(int sockfd);	// Get port number
	void tcpConnect();	// Establish TCP connection with client
	void stringToMatrix(char *buf);	// Make matrix from string
	void printEdgeCost();	// Print out the Edge and Cost based on the matrix
	void printMatrix();	// Print matrix
public:
	Server(string myServer);	// Server class constructor
	virtual ~Server();	// Server class destructor
	void bootUp();	// Boot the server
	void udpServer();	// Open UDP Server
};

#endif	/*_SERVER_H_*/
