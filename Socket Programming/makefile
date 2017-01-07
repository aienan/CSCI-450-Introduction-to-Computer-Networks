all : client server

client : client.cpp
	g++ -o client client.cpp -lsocket -lnsl -lresolv

server : server.cpp
	g++ -o server server.cpp -lsocket -lnsl -lresolv

clean :
	rm -f client server
