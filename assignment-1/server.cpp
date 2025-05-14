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

void errorCheck(int var, std::string msg);

int main()
{
    int serverSocketFd;
    int socketOptReturn;
    int socketBindReturn;
    int socketListenReturn;
    std::string socketCreateMsg = "Socket creation error";
    std::string socketOptMsg = "Socket options error";
    std::string socketBindMsg = "Socket bind error";
    std::string socketListenMsg = "Socket listen error";
    std::string socketAcceptMsg = "Socket accept error";
    std::string socketReciveMsg = "Socket receive error";

    struct sockaddr_in serverAdress;

    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(serverSocketFd, socketCreateMsg);

    int opt = 1;

    socketOptReturn = setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    errorCheck(socketOptReturn, socketOptMsg);

    memset(&serverAdress, 0, sizeof(serverAdress));
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_addr.s_addr = inet_addr(IP_TO_LISTEN_TO);
    serverAdress.sin_port = htons(SERVER_PORT);

    socketBindReturn = bind(serverSocketFd, (struct sockaddr *)&serverAdress, sizeof(serverAdress));
    errorCheck(socketBindReturn, socketBindMsg);

    socketListenReturn = listen(serverSocketFd, 5);
    errorCheck(socketListenReturn, socketListenMsg);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int clientSocket = accept(serverSocketFd, (struct sockaddr *)&client_addr, &addrlen);
        errorCheck(clientSocket, socketAcceptMsg);

        char buffer[1024];
        int reciveBytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        errorCheck(reciveBytes, socketReciveMsg);

        if (reciveBytes > 0)
        {
            buffer[reciveBytes] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            if (strcmp(buffer, "NORMAL_DATA:Hello") == 0)
            {
                std::string serverMsg = "SERVER_ACK:HELLO";
                send(clientSocket, serverMsg.c_str(), serverMsg.length(), 0);
                errorCheck(serverSocketFd, "Something went wrong with send");
                std::cout << "Sent: " << serverMsg << std::endl;
            }
            else if (strcmp(buffer, "SEND_URGENT_REQUEST") == 0)
            {
                char oob_byte;
                int socketUrgentRecv = recv(clientSocket, &oob_byte, 1, MSG_OOB);
                std::cout << "Urgent data" << oob_byte << std::endl;
                if (socketUrgentRecv > 0)
                {
                    std::string serverMsg = "SERVER_URGENT_ACK:" + std::string(1, oob_byte);
                    send(clientSocket, serverMsg.c_str(), serverMsg.length(), 0);
                    std::cout << "Sent: " << serverMsg << std::endl;
                }
                else
                {
                    std::string serverMsg = "SERVER_NO_URGENT_DATA";
                    send(clientSocket, serverMsg.c_str(), serverMsg.length(), 0);
                    std::cout << "Sent: " << serverMsg << std::endl;
                }
            }
            else
            {
                std::string serverMsg = "SERVER_UNKNOWN_COMMAND";
                send(clientSocket, serverMsg.c_str(), serverMsg.length(), 0);
                std::cout << "Sent: " << serverMsg << std::endl;
            }
        }

        close(clientSocket);
    }
    close(serverSocketFd);
    return 0;
}

void errorCheck(int var, std::string msg)
{
    if (var < 0)
    {
        std::cerr << msg << ": " << errno << std::endl;
        close(var);
    }
}