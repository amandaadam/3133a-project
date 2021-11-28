#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>
#include <signal.h>

using namespace Sync;

class SocketThread : Thread
{
private:
    bool terminate;
    Socket& socket;
    Socket& other_socket;
public:
    ~SocketThread()
    {}

    SocketThread(Socket& socket, Socket& other_socket)
    : socket(socket), other_socket(other_socket), terminate(false)
    {}

    void terminateThread()
    {
        terminate = true;
    }

    Socket& GetSocket()
    {
        return socket;
    }
    
    virtual long ThreadMain()
    {
        ByteArray data;
        while (!terminate)
        {
            // Read from this socket
            socket.Read(data);

            // Write to the other socket
            other_socket.Write(data);
        }
        return 0;
    }
};

std::vector<SocketThread*> socketThreads;

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    bool done;
public:
    ServerThread(SocketServer& server)
    : server(server), done(false)
    {}

    ~ServerThread()
    {
        // Cleanup
        std::cout << "Server shutting down" << std::endl;

        // Kill remaining clients
        while(!socketThreads.empty()) {
            std::cout << "Killing client" << std::endl;
            SocketThread* st = socketThreads.back();
            st->terminateThread();
            socketThreads.pop_back();
        }
        done = true;
    }

    void shutdown()
    {
        done = true;
    }

    virtual long ThreadMain()
    {   
        while (!done) {
            try {
                // Wait for a client socket connection
                std::cout << "Server awaiting connection" << std::endl;
                Socket* newConnection = new Socket(server.Accept());
                
                // A reference to this pointer 
                Socket& conn1 = *newConnection;
                conn1.Write(ByteArray("success"));
                std::cout << "1st Connection received. Awaiting second connection." << std::endl;
                // Wait for second connection
                Socket* secondConnection = new Socket(server.Accept());
                Socket& conn2 = *secondConnection;
                std::cout << "2nd Connection received. Initializing chat." << std::endl;
                // Signal to bot clients that they should initialize their readers and writers
                conn2.Write(ByteArray("success"));
                conn1.Write(ByteArray("connected"));
                sleep(1);
                conn2.Write(ByteArray("connected"));
                
                // Initialize socket threads (one for each direction of communication)
                SocketThread* thread1 = new SocketThread(conn1, conn2);
                SocketThread* thread2 = new SocketThread(conn2, conn1);
                // Add socket threads to the queue
                socketThreads.push_back(thread1);
                socketThreads.push_back(thread2);
            } catch (...) {

            }
        }
	    return 1;
    }
};


int main(void)
{
    std::cout << "SE3313 Chat - Server" << std::endl;
	
    // Create our server
    SocketServer server(3000);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();
    
    // Shut down and clean up the server
    server.Shutdown();

}
