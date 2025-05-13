#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#define SERVER_PORT 8080
#define IP_TO_LISTEN_TO "127.0.0.1"
#define DATA_TO_SEND "NORMAL_DATA:Hello"
#define SOCKET_CONNECT_MSG "Socket connect msg"
#define SOCKET_SEND_MSG "Socket send msg"

void errorCheck(int var, char *msg)
{
    if (var < 0)
    {
        std::cerr << msg << ": " << errno << std::endl;
    }
}

int main()
{
    int clientSocket;
    int socketConnectReturn;
    int socketSendReturn;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::string message_to_send = DATA_TO_SEND;

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, IP_TO_LISTEN_TO, &serverAddress.sin_addr) <= 0)
    {
        std::cerr << "Invalid address or not supported" << std::endl;
        close(clientSocket);
        return 0;
    }

    socketConnectReturn = connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    errorCheck(socketConnectReturn, SOCKET_CONNECT_MSG);

    socketSendReturn = send(clientSocket, message_to_send.c_str(), message_to_send.length(), 0);
    errorCheck(socketSendReturn, SOCKET_SEND_MSG);

    std::cout << "Sent to registry server: " << message_to_send << std::endl;

    close(clientSocket);

    // int clientFd;
    // struct sockaddr_in registryServerAddr;
    // int registryServerInet;
    // int socketConnect;
    // std::string my_server_ip = MY_SERVER_IP_STRING;
    // int my_server_port = STUDENT_SERVER_PORT;
    // std::string message_to_send = my_server_ip + ":" + std::to_string(my_server_port);
    // int socketSend;
    // char reciveBuffer[1024];

    // clientFd = socket(AF_INET, SOCK_STREAM, 0);
    // if (clientFd < 0)
    // {
    //     std::cerr << "Socket creation error" << std::endl;
    // }
    // memset(&registryServerAddr, 0, sizeof(registryServerAddr));
    // registryServerAddr.sin_family = AF_INET;
    // registryServerAddr.sin_port = htons(INSTRUCTOR_REGISTRY_PORT);
    // registryServerInet = inet_pton(AF_INET, INSTRUCTOR_REGISTRY_IP, &registryServerAddr.sin_addr);
    // if (registryServerInet == 0)
    // {
    //     std::cerr << "Registry server ip conversion failed" << std::endl;
    // }

    // socketConnect = connect(clientFd, (struct sockaddr *)&registryServerAddr, sizeof(registryServerAddr));

    // if (socketConnect < 0)
    // {
    //     std::cerr << "Connection failed" << std::endl;
    //     close(clientFd);
    // }

    // socketSend = send(clientFd, message_to_send.c_str(), message_to_send.length(), 0);
    // if (socketSend == -1)
    // {
    //     std::cerr << "Failed to send" << std::endl;
    // }
    // std::cout << "Sent to Registry Server: " << message_to_send << std::endl;
    // recv(clientFd, &reciveBuffer, sizeof(reciveBuffer) - 1, 0);
    // close(clientFd);

    return 0;
}