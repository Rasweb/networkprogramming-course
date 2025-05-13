#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SERVER_PORT 8080
#define REQUEST_BACKLOG 5
#define IP_TO_LISTEN_TO "127.0.0.1"
#define SOCKET_CREATE_MSG "Socket creation error"
#define SOCKET_OPT_MSG "Socket options error"
#define SOCKET_BIND_MSG "Socket bind error"
#define SOCKET_LISTEN_MSG "Socket listen error"
#define SOCKET_ACCEPT_MSG "Socket accept error"
#define SOCKET_RECEIVE_MSG "Socket receive error"

void errorCheck(int var, char *msg)
{
    if (var < 0)
    {
        std::cerr << msg << ": " << errno << std::endl;
    }
}

int main()
{
    int serverSocketFd;
    int socketOptReturn;
    int socketBindReturn;
    int socketListenReturn;

    struct sockaddr_in serverAdress;

    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(serverSocketFd, SOCKET_CREATE_MSG);

    int opt = 1;

    int socketOptReturn = setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    errorCheck(socketOptReturn, SOCKET_OPT_MSG);

    memset(&serverAdress, 0, sizeof(serverAdress));
    serverAdress.sin_family = AF_INET;
    serverAdress.sin_addr.s_addr = inet_addr(IP_TO_LISTEN_TO);
    serverAdress.sin_port = htons(SERVER_PORT);

    socketBindReturn = bind(serverSocketFd, (struct sockaddr *)&serverAdress, sizeof(serverAdress));
    errorCheck(socketBindReturn, SOCKET_BIND_MSG);

    socketListenReturn = listen(serverSocketFd, 5);
    errorCheck(socketListenReturn, SOCKET_LISTEN_MSG);

    while (1)
    {
        int clientSocket = accept(serverSocketFd, NULL, NULL);
        errorCheck(clientSocket, SOCKET_ACCEPT_MSG);

        char buffer[1024];
        int reciveBytes = recv(clientSocket, buffer, sizeof(buffer), 0);
        errorCheck(reciveBytes, SOCKET_RECEIVE_MSG);

        if (reciveBytes > 0)
        {
            buffer[reciveBytes] = '\0';
            std::cout << "Received from client: " << buffer << std::endl;
        }

        close(serverSocketFd);
    }
    // int serverFd;
    // int socketOptions;
    // struct sockaddr_in serverAddr;
    // int socketBind;
    // int socketListen;
    // char instructorBuffer[1024];

    // serverFd = socket(AF_INET, SOCK_STREAM, 0);
    // if (serverFd < 0)
    // {
    //     std::cerr << "Socket creation error" << std::endl;
    // }

    // int optval = 1;
    // socketOptions = setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optval,
    //                            sizeof(optval));
    // if (socketOptions < 0)
    // {
    //     std::cerr << "Failed to set socket options" << std::endl;
    // }

    // memset(&serverAddr, 0, sizeof(serverAddr));
    // serverAddr.sin_family = AF_INET;
    // serverAddr.sin_addr.s_addr = INADDR_ANY;
    // serverAddr.sin_port = htons(STUDENT_SERVER_PORT);

    // socketBind = bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    // if (socketBind < 0)
    // {
    //     std::cerr << "Socket binding failed" << std::endl;
    //     close(serverFd);
    // }

    // socketListen = listen(serverFd, REQUEST_BACKLOG);
    // if (socketListen < 0)
    // {
    //     std::cerr << "Socket listen failed" << std::endl;
    //     close(serverFd);
    // }

    // std::cout << "Student server listening on port " << STUDENT_SERVER_PORT << "..." << std::endl;

    // while (1)
    // {
    //     struct sockaddr_in instructor_addr;
    //     socklen_t instructor_addr_len = sizeof(instructor_addr);
    //     int instructor_conn_fd = accept(serverFd, (struct sockaddr *)&instructor_addr, &instructor_addr_len);
    //     if (instructor_conn_fd < 0)
    //     {
    //         std::cerr << "Accept failed" << std::endl;
    //         close(instructor_conn_fd);
    //     }

    //     char instructor_ip_str[INET_ADDRSTRLEN];
    //     inet_ntop(AF_INET, &instructor_addr.sin_addr, instructor_ip_str, INET_ADDRSTRLEN);
    //     std::cout << "Connection accepted from instructor's server at " << instructor_ip_str << ":" << ntohs(instructor_addr.sin_port) << std::endl;
    //     recv(instructor_conn_fd, &instructorBuffer, sizeof(instructorBuffer) - 1, 0);
    //     close(instructor_conn_fd);
    //     close(serverFd);

    //     /* code */
    // }

    return 0;
}