#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>

#define INSTRUCTOR_REGISTRY_IP "172.16.217.188"
#define INSTRUCTOR_REGISTRY_PORT 8080
#define STUDENT_SERVER_PORT 14623

int main()
{

    int clientFd;
    struct sockaddr_in registryServerAddr;
    int registryServerInet;
    int socketConnect;
    std::string message_to_send = "HEJ HEJ";
    int socketSend;
    char reciveBuffer[1024];

    clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientFd < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
    }
    memset(&registryServerAddr, 0, sizeof(registryServerAddr));
    registryServerAddr.sin_family = AF_INET;
    registryServerAddr.sin_port = htons(INSTRUCTOR_REGISTRY_PORT);
    registryServerAddr.sin_addr.s_addr = INADDR_ANY;
    // registryServerInet = inet_pton(AF_INET, INSTRUCTOR_REGISTRY_IP, &registryServerAddr.sin_addr);
    if (registryServerInet == 0)
    {
        std::cerr << "Registry server ip conversion failed" << std::endl;
    }

    socketConnect = connect(clientFd, (struct sockaddr *)&registryServerAddr, sizeof(registryServerAddr));

    if (socketConnect < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        close(clientFd);
    }

    socketSend = send(clientFd, message_to_send.c_str(), message_to_send.length(), 0);
    if (socketSend == -1)
    {
        std::cerr << "Failed to send" << std::endl;
    }
    std::cout << "Sent to Registry Server: " << message_to_send << std::endl;
    recv(clientFd, &reciveBuffer, sizeof(reciveBuffer) - 1, 0);
    close(clientFd);

    return 0;
}