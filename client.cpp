#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

//prototype of command line interface handler function
bool CLI_Handler(const std::string& , std::string& , std::string&);

int main()
{
		//initializing socket file descriptor
		int sock = socket(AF_INET , SOCK_STREAM , 0);

		//if something goes wrong
		if(sock == -1)
		{
				std::cerr << "Error creating socket , reinstall application";
				return 1;
		}

		std::cout << "Hello , welcome to our client application!" << std::endl;
		std::cout << "In our application supported commands are ..." << std::endl;
		std::cout << "Use - Connect <IP_ADRESS> <PORT> for connecting to the server..." << std::endl;
		std::cout << "get_time - to ask server for time..." << std::endl;
		std::cout << "disconnect - for disconnecting..." << std::endl;
		std::cout << "Note that when you aren't connected with server, get_time and disconnect won't work!\n";
		std::cout << "Local IP address is 0.0.0.0 , port - 7821. Other arguments might not work" << std::endl;
		
		//our parameters we need to know
		std::string user_input;
		std::string ipv4;
		std::string portNumber;

		//waiting still user will input "connect" and after some ip address and port
		bool isInputRight = false;
		std::cout << "If you want to just leave this application, just type 'exit' " << std::endl;
		while(!isInputRight)
		{
				std::getline(std::cin , user_input);
				if(user_input == "exit")
				{
						return 0;
				}
				isInputRight = CLI_Handler(user_input , ipv4 , portNumber);
		}

		//creating struct for client(see server file for more information)
		sockaddr_in client;

		//initializing it to IPv4 parameter, and given from user's input
	
		client.sin_family = AF_INET;
		client.sin_port = htons(std::atoi(portNumber.c_str()));
		client.sin_addr.s_addr = inet_addr(ipv4.c_str());
		std::cout << "Trying to connect with server..." << std::endl;
			
		int connectResult = connect(sock , (sockaddr*)&client , sizeof(client));
	
		//if something goes wrong
		if(connectResult == -1)
		{
				std::cout << "Connection fault\n";
				return 1;
		}

		std::cout << "Connected to the server " << std::endl;

		//basic ingredients for communication
		char buffer[2000];
		char disc_accept[] = "$";
		std::string input;
		std::string server_response;
	
		//how much information we recieved
		int bytesRecieved; 
		
		std::cout << "Welcome to the server!" << std::endl;
		memset(buffer , 0 , 2000);
		std::cout << "our server commands - get_time - returns current time , disconnect - disconnects you from the server" << std::endl;
		while(sock > 0)
		{
				//client enters his commands here
				std::cout << "Client: ";
				std::getline(std::cin , input);

				//it will be sent to server, handled and...
				send(sock , input.c_str() , input.size() + 1, 0);

				//it will be return to us as of the information we need from server
				memset(buffer , 0 , 2000);
				bytesRecieved = recv(sock , buffer , 2000 , 0);
				if(bytesRecieved < 0)
				{
						std::cout << "Connection lost, disconnecting from the server..." << std::endl;
						break;
				}
				if(buffer == disc_accept)
				{
						std::cout << "Thank you for choosing our server. Goodbye!" << std::endl;
				}
				std::cout << "Server: " << std::string(buffer , bytesRecieved) << std::endl;
		}

		//close file descriptor
		close(sock);
		return 0;
}

bool CLI_Handler(const std::string& input , std::string& ip , std::string& port)
{
		//first command of our users
		std::string command;

		//checking is first command about connecting to server or not
		int first_space = input.find(' ');
		command = input.substr(0 , first_space);

		//if it's true we will seperate user's input to details we need
		if(command == "Connect" || command == "connect")
		{
				int second_space = input.find(' ' , first_space + 1);
				ip = input.substr(first_space + 1 , second_space - first_space - 1);
				int third_space = input.find(' ' , second_space + 1);
				port = input.substr(second_space + 1 , third_space - second_space);
				std::cout << ip << " " << port << std::endl;
				return true;
		}
		//our application is about connecting, and communicating with server!
				std::cout << "Unknown Command" << std::endl;
				return false;
	}
