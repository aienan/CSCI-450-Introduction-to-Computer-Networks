#ifndef _CLIENT_H_
#define _CLIENT_H_

// #define HOST_NAME "nunki.usc.edu"
#define HOST_NAME "localhost"
#define SERVER_NUM 4
#define USC_ID 738
#define SERVER_A_UDP_PORT (21000+USC_ID)
#define SERVER_B_UDP_PORT (22000+USC_ID)
#define SERVER_C_UDP_PORT (23000+USC_ID)
#define SERVER_D_UDP_PORT (24000+USC_ID)
#define CLIENT_TCP_PORT (25000+USC_ID)
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

class Client {
	class Node {
	public :
		int row;
		int col;
		int cost;
		Node(){}
		Node(int row, int col, int cost){
			this->row = row;
			this->col = col;
		}
		virtual ~Node(){}
		void set(int row, int col, int cost){
			this->row = row;
			this->col = col;
			this->cost = cost;
		}
	};
	int matrix[SERVER_NUM][SERVER_NUM];	// Adjacency matrix
	struct addrinfo *clientAddrInfo;	// struct addrinfo of client
	char serverChar[SERVER_NUM];	// Server's character such as A, B, C or D
	
	string getIP(struct addrinfo *addrInfo);	// Get IP address
	unsigned short int getPort(int sockfd);	// Get port number
	void dnsWork();	// Do the DNS work
	string matrixString();	// Convert matrix into stringstream
	void printEdgeCost();	// Print out the Edge and Cost based on the matrix
	void tcpServer();	// Open TCP Server
	void minimumSpanningTree();	// Calculate minimum spanning tree
	int minSelect(int index, int selectNum, int edgeNum, Node *edgeArr, int *selectArr);
public:
	Client();	// Client class constructor
	virtual ~Client();	// Client class destructor
	void bootUp();	// Boot the client
	void udpConnect();	// Send data to servers
};

#endif	/*_CLIENT_H_*/
