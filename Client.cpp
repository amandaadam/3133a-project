
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

bool done = false;

class WriteThread : Thread
{
private:
	Socket& socket;
public:
	~WriteThread()
	{
		std::cout << "Write thread closed" << std::endl;
	}
	
	WriteThread(Socket& socket)
	: socket(socket)
	{}

	virtual long ThreadMain()
	{
		std::string input;
		while(true)
		{
			// Attempt to read from socket
			std::cin >> input;
			ByteArray data = ByteArray(input);
			socket.Write(data);
			if (input == "done") {
				done = true;
				break;
			}
			std::cout << "You: " + input << std::endl;
		}
		return 1;
	}
};

class ReadThread : Thread
{
private:
	Socket& socket;
public:
	ReadThread(Socket& socket)
	: socket(socket)
	{}

	~ReadThread()
	{
		std::cout << "Read thread closed" << std::endl;
	}

	virtual long ThreadMain()
	{
		while(true)
		{
			// Attempt to read from socket
			ByteArray data;
			socket.Read(data);
			std::string data_str = data.ToString();
			if (data_str == "done") {
				done = true;
				break;
			}
			// Display result
			std::cout << "Other Client: " + data_str << std::endl;
		}
		return 1;
	}
};

int main(void)
{
	// Welcome the user 
	std::cout << "SE3313 Lab 3 Client" << std::endl;

	// Create our socket
	Socket socket("127.0.0.1", 3000);

	socket.Open();

	// Connection confirmation
	ByteArray data;
	socket.Read(data);
	std::string data_str = data.ToString();
	if (data_str == "success") {
		// Wait for other client to connect
		std::cout << "Waiting for other client to connect..." << std::endl;
		socket.Read(data);
		data_str = data.ToString();
		if (data_str != "connected") {
			return 0;
		}
	} else {
		return 0;
	}
	std::cout << "Client connected!" << std::endl;

	WriteThread* wt = new WriteThread(socket);
	ReadThread* rt = new ReadThread(socket);
	while (!done)
	{
		sleep(1);
	}
	/*
	// Get user input
	std::string input;
	while(true) {
		std::cout << "Enter a message to send: ";
		std::cin >> input;

		if (input == "done") {
			break;
		}

		ByteArray data = ByteArray(input);

		if (socket.Write(data) <= 0 || socket.Read(data) <= 0) {
			break;
		}

		// Convert to string and display
		std::string str_result = data.ToString();

		// Allow server to kill client
		if (str_result == "done") {
			break;
		}
		std::cout << str_result << std::endl;
	}
	*/

	socket.Close();

	return 0;
}
