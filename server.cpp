#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <string>
#include <pthread.h>

//prototype for user input handler function
int Command_Handler(std::string);

//prototype for seperating clients, so that different threads work
void* Client_Servant(void*);

int client_id = 0;

int main()
{
		int listening_sock , client_sock , *new_socket;
	
		//creating socket desc, initializing it for ipv4 - AF_INET
		listening_sock = socket(AF_INET , SOCK_STREAM , 0);
	
		//if something goes wrong
		if(listening_sock == -1)
		{
				std::cerr << "unable to create socket descriptor   ";
				return 1;
		}
		//	sockaddr_in is IPv4 AF_INET , or IPv6 ... sockets structure
		sockaddr_in server;

		//choosing ipv4 or ipv6 , i choosed ipv4
		server.sin_family = AF_INET;

		//server.sin_addr.s_addr = INADDR_ANY;
		//port number in the range of which the server will work
		int port = 7821;
		server.sin_port = htons(port);

		inet_pton(AF_INET , "0.0.0.0" , &server.sin_addr);


		//this function convers char* into unsigned long as I guess, it's member of server.sin_addr.s_addr
		//sin_addr of sockaddr_in structure is also struct

		//binding our socket with structure with what we will communicate to our clients
		if(bind(listening_sock , (sockaddr*)&server , sizeof(server)) == -1)
		{
				const int optional = 1;
				const socklen_t optional_length = sizeof(optional);
				setsockopt(listening_sock , SOL_SOCKET , SO_REUSEPORT , &optional , optional_length);
				close(listening_sock);
				std::cerr << "bind failed, restard the programm  ";
				shutdown(listening_sock , 2);
				return 1;
		}
		
		//the maximum value of second argument is 5, I haven't read man yet
		if(listen(listening_sock , 5) == -1)
		{
				close(listening_sock);
				std::cerr << "Listening was aborted";
				return 1;
		}
	
		//client structure , accept will keep in touch these objects, cause we binded listening socket to server
		sockaddr_in client;
		int size = sizeof(struct sockaddr_in);
		//size of it as argument for accept
		
		std::cout << "Server is ready to work!" << std::endl;	

		//this cycle gonna work forever waiting for clients
		while(client_sock = accept(listening_sock , (struct sockaddr*)&client , (socklen_t*)&size))
		{	
				++client_id;
				//'C' thread destination
				pthread_t user_thread;

				//casting allocated memory to int*
				new_socket = (int*)malloc(1);

				//assigning it to client_socket , so we can after this accept more client sockets
				*new_socket = client_sock;

				//creating a thread, telling it which function need work , and it's argument
				//what need to be casted into void* , 'C' threads are working only with that
				if(pthread_create(&user_thread , NULL , Client_Servant , (void*)new_socket) < 0)
				{	
						close(listening_sock);
						close(client_sock);
						perror("Couldn't create thread: ");
						return 1;
				}
		}

		//at the end it will close files

		const int optional = 1;
		const socklen_t optional_length = sizeof(optional);
		setsockopt(listening_sock , SOL_SOCKET , SO_REUSEPORT , &optional , optional_length);
		close(client_sock);
		close(listening_sock);
		return 0;
}


int Command_Handler(std::string user_command)
{
		std::string command1 = "get_time";
		std::string command2 = "disconnect";
		std::string command3_1 = "connect";
		std::string command3_2 = "Connect";
		if(user_command == command1)
		{
				return 1;
		}
		else if(user_command == command2)
		{
				return 2;
		}
		else if(user_command == command3_1 || user_command == command3_2)
		{
				return 3;
		}
		else
				return 0;
}

void* Client_Servant(void* socket)
{
		int current_client_id = client_id;
		std::cout << "A new client connected to the server! | ID:" << current_client_id << std::endl;
		int sock = *(int*)socket;
		char buffer[2000];
		char unknown_command_str[] = "Unknown Command\n";
		char double_connect_case_str[] = "Error! You're already connected!\n";
		time_t now = time(NULL);
		while(sock > 0)
		{
				memset(buffer , 0 , 2000);

				int bytesRecv = recv(sock , buffer , 2000 , 0);
		
				if(bytesRecv == -1)
				{
						std::cout << "There is connection issue";
						return 0;
				}

				std::cout << "Client ID[" << current_client_id << "] inputs: " << buffer << " | " << strlen(buffer) << " symbols " << std::endl;

				int command_id = Command_Handler(std::string(buffer , 0 , bytesRecv));
				if(command_id == 1)
				{
						//basic 'C' struct where is anything about time

						struct tm *cur_time = localtime(&now);
						char time_buffer[20];
						memset(time_buffer , 0 , 20);

						//here as 3 argument i tell this function , in what format I'd like to see the time format
						strftime(time_buffer , 20 , "%Y-%m-%d %H:%M:%S" , cur_time);

						//sending time information to client
						send(sock , time_buffer , strlen(time_buffer), 0);
				}
				else if(command_id == 2)
				{
						//client decided to disconnect

						std::cout << "Client ID[" << current_client_id << "] disconnecting..." << std::endl;
						char disc_allow_str[] = "disconnect accepted, disconnecting you from server...\nPress double enter to exit from programm";
						send(sock , disc_allow_str , strlen(disc_allow_str) , 0);
						close(sock);
						return 0;
				}
				else if(command_id == 3)
				{
						// in case when client will try to repeat connect command, just for fun

						send(sock , double_connect_case_str , strlen(double_connect_case_str) , 0);
						std::cout << "Warning! Client ID[" << current_client_id << "] is trying to connect while being connected!" << std::endl;
				}	
				else
				{
						//other cases will return client this
						send(sock , unknown_command_str , strlen(unknown_command_str) , 0);
						std::cout << "Warning! Client ID[" << current_client_id << "] inserted unknown command by server!" << std::endl;
				}
		}

		//we need to free allocated memory
		free(socket);
		return 0;
}
