#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#define SERVER_PORT 8080
#define REQUEST_BACKLOG 5
#define IP_TO_LISTEN_TO "127.0.0.1"

int socketServerConf(const char *createMsg, const char *sockOptMsg, const char *sockBindMsg, const char *sockListenMsg);
void errorCheck(int var, const char *msg);
void socketSendHandler(std::string sendMsg, int clientSock, std::string errorMsg);
int main()
{
    int serverSocketFd;
    std::string normalRequestMsg = "SERVER_ACK:Hello";
    std::string UrgentRequestMsg = "SERVER_URGENT_ACK:";
    std::string noUrgentData = "SERVER_NO_URGENT_DATA";
    std::string unknownRequest = "SERVER_UNKNOWN_COMMAND";

    const char *socketCreateMsg = "Socket creation, error type";
    const char *socketOptMsg = "Socket options, error type";
    const char *socketBindMsg = "Socket bind, error type";
    const char *socketListenMsg = "Socket listen, error type";
    const char *socketAcceptMsg = "Socket accept, error type";
    const char *socketReciveMsg = "Socket receive, error type";
    const char *socketSendNormalMsg = "Socket send normal request, error type";
    const char *socketSendUrgentMsg = "Socket send urgent request, error type";
    const char *socketSendNoUrgentMsg = "Socket send no urgent data, error type";
    const char *socketUnknownMsg = "Socket send unknown request, error type";

    serverSocketFd = socketServerConf(socketCreateMsg, socketOptMsg, socketBindMsg, socketListenMsg);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);

        int clientSocket = accept(serverSocketFd, (struct sockaddr *)&client_addr, &addrlen);
        errorCheck(clientSocket, socketAcceptMsg);
        std::cout << "\n"
                  << "Client connected";
        std::cout << std::endl;
        while (1)
        {
            char buffer[1024];
            int reciveBytes = recv(clientSocket, buffer, sizeof(buffer), 0);
            errorCheck(reciveBytes, socketReciveMsg);

            if (reciveBytes > 0)
            {
                buffer[reciveBytes] = '\0';
                std::cout << "Received: " << buffer << std::endl;

                if (strcmp(buffer, "NORMAL_DATA:Hello") == 0)
                {
                    socketSendHandler(normalRequestMsg, clientSocket, socketSendNormalMsg);
                }
                else if (strcmp(buffer, "SEND_URGENT_REQUEST") == 0)
                {
                    char oob_byte;
                    int socketUrgentRecv = recv(clientSocket, &oob_byte, 1, MSG_OOB);
                    if (socketUrgentRecv > 0)
                    {
                        std::string serverMsg = UrgentRequestMsg + std::string(1, oob_byte);
                        socketSendHandler(serverMsg, clientSocket, socketSendUrgentMsg);
                    }
                    else
                    {
                        int trailingBytes = recv(clientSocket, buffer, sizeof(buffer), 0);
                        // if (trailingBytes > 0)
                        // {
                        //     buffer[trailingBytes] = '\0';
                        //     std::string serverMsg = "NORMAL_DATA:" + std::string(buffer);
                        //     socketSendHandler(serverMsg, clientSocket, "Normal data trailing, error type");
                        // }
                        if (trailingBytes == 0)
                        {
                            std::cout << "Client disconnected." << std::endl;
                            std::cout << std::endl;
                            break;
                        }
                        {
                            socketSendHandler(noUrgentData, clientSocket, socketSendNoUrgentMsg);
                        }
                    }
                }
                else
                {
                    socketSendHandler(unknownRequest, clientSocket, socketUnknownMsg);
                }
            }
            else if (reciveBytes == 0)
            {
                std::cout << "Client disconnected" << std::endl;
                std::cout << std::endl;

                break;
            }
        }
        close(clientSocket);
        std::cout << "Waiting for new connection..." << std::endl;
    }
    close(serverSocketFd);
    return 0;
}

void errorCheck(int var, const char *msg)
{
    if (var < 0)
    {
        std::cerr << "Something went wrong with: ";
        perror(msg);
        close(var);
    }
}

void socketSendHandler(std::string sendMsg, int clientSock, std::string errorMsg)
{
    send(clientSock, sendMsg.c_str(), sendMsg.length(), 0);
    errorCheck(clientSock, errorMsg.c_str());
    std::cout << "Sent: " << sendMsg << std::endl;
    std::cout << std::endl;
}

int socketServerConf(const char *createMsg, const char *sockOptMsg, const char *sockBindMsg, const char *sockListenMsg)
{
    int socketFd;
    int optReturn;
    int bindReturn;
    int listenReturn;
    struct sockaddr_in serverAdress;

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(socketFd, createMsg);

    int opt = 1;

    optReturn = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    errorCheck(optReturn, sockOptMsg);

    memset(&serverAdress, 0, sizeof(serverAdress));
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_addr.s_addr = inet_addr(IP_TO_LISTEN_TO);
    serverAdress.sin_port = htons(SERVER_PORT);

    bindReturn = bind(socketFd, (struct sockaddr *)&serverAdress, sizeof(serverAdress));
    errorCheck(bindReturn, sockBindMsg);

    listenReturn = listen(socketFd, 5);
    errorCheck(listenReturn, sockListenMsg);

    return socketFd;
}