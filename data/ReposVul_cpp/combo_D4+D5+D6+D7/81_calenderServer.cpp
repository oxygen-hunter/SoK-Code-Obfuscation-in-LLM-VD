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

using namespace std;

struct Args { int argc; char** argv; };
struct AddrInfo { struct addrinfo hints; struct addrinfo* serverInfo; };
struct Buffers { char requestBuffer[513]; string request; string response; };

int listenSocket;
int clientSocket;
char portBuf[6];
Args args;
AddrInfo addrInfo;
Buffers buffers;
list<thread> clientThreads;
thread ttlThread(expireSessionIDs);

int main(int argc, char** argv) {
    args = {argc, argv};

    if (args.argc != 2)
        printUsageAndExit();

    struct PortNum {
        int portNum;
        PortNum() : portNum(0) {}
        void set(int num) { portNum = num; }
    } portNum;

    try {
        portNum.set(stoi(args.argv[1]));
    } catch (invalid_argument e) {
        cerr << "Not a valid number!" << endl;
        printUsageAndExit();
    } catch (out_of_range e) {
        cerr << "Port must be between 1 and 65535!" << endl;
        printUsageAndExit();
    }

    if (portNum.portNum < 1 || portNum.portNum > 65535) {
        cerr << "Port must be between 1 and 65535!" << endl;
        printUsageAndExit();
    }

    init();
    serverListen(portNum.portNum);
    shutdown();
}

void printUsageAndExit() {
    cerr << "Usage: ./calenderServer portNum, where portNum is the number of the port to listen on. " << endl;
    exit(1);
}

void serverListen(int portNum) {
    memset(&addrInfo.hints, 0, sizeof(addrInfo.hints));
    addrInfo.hints.ai_family = AF_UNSPEC;
    addrInfo.hints.ai_socktype = SOCK_STREAM;
    addrInfo.hints.ai_flags = AI_PASSIVE;

    sprintf(portBuf, "%d", portNum);

    if (getaddrinfo(NULL, portBuf, &addrInfo.hints, &addrInfo.serverInfo) != 0) {
        cerr << "Problem getting address info!" << endl;
        exit(1);
    }

    if ((listenSocket = socket(addrInfo.serverInfo->ai_family, addrInfo.serverInfo->ai_socktype, addrInfo.serverInfo->ai_protocol)) == -1) {
        cerr << "Couldn't get a system socket!" << endl;
        exit(1);
    }
    openSockets.push_front(listenSocket);
    int yes = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(listenSocket, addrInfo.serverInfo->ai_addr, addrInfo.serverInfo->ai_addrlen) == -1) {
        cerr << "Couldn't bind the socket!" << endl;
        exit(1);
    }

    freeaddrinfo(addrInfo.serverInfo);

    if (listen(listenSocket, 10) == -1) {
        cerr << "Couldn't listen on socket!" << endl;
        exit(1);
    }

    listenLoop(listenSocket);
}

void listenLoop(int listenSocket) {
    struct sockaddr_storage clientAddr;
    socklen_t cAddrSize = sizeof(clientAddr);

    ttlThread = thread(expireSessionIDs);

    while (true) {
        if ((clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &cAddrSize)) == -1) {
            cerr << "Failed to accept client." << endl;
            continue;
        }
        openSockets.push_front(clientSocket);
        clientThreads.push_front(thread(handleClient, clientSocket));
    }
}

void expireSessionIDs() {
    while (true) {
        for (auto sessionTTLMapping : sessionTTLs) {
            if (sessionTTLMapping.second-- <= 0) {
                sessionMap.erase(sessionTTLMapping.first);
                sessionTTLs.erase(sessionTTLMapping.first);
            }
        }
        sleep(1);
    }
}

void handleClient(int clientSocket) {
    unsigned bytesSent;
    int sending, bytesReceived;

    bytesReceived = recv(clientSocket, buffers.requestBuffer, 513, 0);
    while (bytesReceived == 513) {
        buffers.request.append(buffers.requestBuffer, bytesReceived);
        bytesReceived = recv(clientSocket, buffers.requestBuffer, 513, MSG_DONTWAIT);
    }
    if (bytesReceived != -1) {
        buffers.request.append(buffers.requestBuffer, bytesReceived);
    }
    if (bytesReceived == 0) return;

    try {
        buffers.response = handleRequest(buffers.request);
    } catch (exception e) {
        cerr << "got an exception, killing client thread..." << endl;
        return;
    }

    bytesSent = 0;
    while (bytesSent < buffers.response.length()) {
        sending = send(clientSocket, buffers.response.c_str(), buffers.response.length(), 0);
        if (sending == -1) {
            cerr << "Failed to respond to client. " << endl;
            break;
        }
        bytesSent += sending;
    }

    close(clientSocket);
    openSockets.remove(clientSocket);
}

string handleRequest(const string& request) {
    map<string, string>* reqHeaders = parseRequest(request);
    string response;

    if ((*reqHeaders)["method"].compare("GET") == 0) {
        response = handleGet(reqHeaders);
    } else if ((*reqHeaders)["method"].compare("POST") == 0) {
        response = handlePost(reqHeaders);
    } else {
        string message = "404 Page Not Found";
        cout << "returning error, method is '" << (*reqHeaders)["method"] << "'" << endl;
        response = getResponseHeader("HTTP/1.1 404 Not Found", reqHeaders, message.size()) + "\r\n" + message;
    }

    delete reqHeaders;
    return response;
}