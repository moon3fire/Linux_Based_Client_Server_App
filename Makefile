CFLAGS = -g -pthread
all: Client_app Server_app
	

Server_app:			server.cpp
			g++ -o Server_app $(CFLAGS) server.cpp

Client_app:			client.cpp
			g++ -o Client_app $(CFLAGS) client.cpp
