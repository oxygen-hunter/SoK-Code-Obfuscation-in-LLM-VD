#include "calenderServer.h"
#include "serverActions.h"
#include "persistentState.h"
#include "renderPage.h"
#include "objectCache.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <thread>
#include <algorithm>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include <Python.h>
#include <dlfcn.h>

using namespace std;

extern "C" {
    void init();
    void shutdown();
    string handleRequest(const string&);
}

int main(int argc, char** argv){
    if (argc != 2) {
        cerr << "Usage: ./calenderServer portNum, where portNum is the number of the port to listen on. " << endl;
        exit(1);
    }

    int portNum;
    try {
        portNum = stoi(argv[1]);
        if (portNum < 1 || portNum > 65535) throw out_of_range("Invalid range");
    } catch (...) {
        cerr << "Port must be a number between 1 and 65535!" << endl;
        exit(1);
    }

    void *handle = dlopen("./libserver.so", RTLD_LAZY);
    if (!handle) {
        cerr << "Failed to load library: " << dlerror() << endl;
        exit(1);
    }

    auto init = (void (*)())dlsym(handle, "init");
    auto shutdown = (void (*)())dlsym(handle, "shutdown");
    auto handleRequest = (string (*)(const string&))dlsym(handle, "handleRequest");

    if (!init || !shutdown || !handleRequest) {
        cerr << "Failed to load functions: " << dlerror() << endl;
        dlclose(handle);
        exit(1);
    }

    init();
    serverListen(portNum);
    shutdown();

    dlclose(handle);
}

void serverListen(int portNum) {
    struct addrinfo hints, *serverInfo;
    char portBuf[6];
    int listenSocket;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    sprintf(portBuf, "%d", portNum);

    if (getaddrinfo(NULL, portBuf, &hints, &serverInfo) != 0) {
        cerr << "Problem getting address info!" << endl;
        exit(1);
    }

    if ((listenSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) == -1) {
        cerr << "Couldn't get a system socket!" << endl;
        exit(1);
    }

    int yes = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(listenSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
        cerr << "Couldn't bind the socket!" << endl;
        exit(1);
    }

    freeaddrinfo(serverInfo);

    if (listen(listenSocket, 10) == -1) {
        cerr << "Couldn't listen on socket!" << endl;
        exit(1);
    }

    listenLoop(listenSocket);
}

void listenLoop(int listenSocket) {
    struct sockaddr_storage clientAddr;
    socklen_t cAddrSize = sizeof(clientAddr);
    list<thread> clientThreads;

    while (true) {
        int clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &cAddrSize);
        if (clientSocket == -1) {
            cerr << "Failed to accept client." << endl;
            continue;
        }
        clientThreads.push_front(thread(handleClient, clientSocket));
    }
}

void handleClient(int clientSocket) {
    const int BUFFERSIZE = 513;
    char requestBuffer[BUFFERSIZE];
    string request, response;
    int bytesReceived;

    bytesReceived = recv(clientSocket, requestBuffer, BUFFERSIZE, 0);
    while (bytesReceived == BUFFERSIZE) {
        request.append(requestBuffer, bytesReceived);
        bytesReceived = recv(clientSocket, requestBuffer, BUFFERSIZE, MSG_DONTWAIT);
    }
    if (bytesReceived != -1) {
        request.append(requestBuffer, bytesReceived);
    }
    if (bytesReceived == 0) return;

    try {
        response = handleRequest(request);
    } catch (exception &e) {
        cerr << "Exception caught, closing client thread..." << endl;
        return;
    }

    unsigned bytesSent = 0;
    while (bytesSent < response.length()) {
        int sending = send(clientSocket, response.c_str(), response.length(), 0);
        if (sending == -1) {
            cerr << "Failed to respond to client. " << endl;
            break;
        }
        bytesSent += sending;
    }

    close(clientSocket);
}