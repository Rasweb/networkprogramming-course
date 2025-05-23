#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define STUDENT_SERVER_PORT 14623
#define REQUEST_BACKLOG 5

int main()
{
    int serverFd;
    int socketOptions;
    struct sockaddr_in serverAddr;
    int socketBind;
    int socketListen;
    char instructorBuffer[1024];

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
    }

    int optval = 1;
    socketOptions = setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optval,
                               sizeof(optval));
    if (socketOptions < 0)
    {
        std::cerr << "Failed to set socket options" << std::endl;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(STUDENT_SERVER_PORT);

    socketBind = bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (socketBind < 0)
    {
        std::cerr << "Socket binding failed" << std::endl;
        close(serverFd);
    }

    socketListen = listen(serverFd, REQUEST_BACKLOG);
    if (socketListen < 0)
    {
        std::cerr << "Socket listen failed" << std::endl;
        close(serverFd);
    }

    std::cout << "Student server listening on port " << STUDENT_SERVER_PORT << "..." << std::endl;

    while (1)
    {
        struct sockaddr_in instructor_addr;
        socklen_t instructor_addr_len = sizeof(instructor_addr);
        int instructor_conn_fd = accept(serverFd, (struct sockaddr *)&instructor_addr, &instructor_addr_len);
        if (instructor_conn_fd < 0)
        {
            std::cerr << "Accept failed" << std::endl;
            close(instructor_conn_fd);
        }

        char instructor_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &instructor_addr.sin_addr, instructor_ip_str, INET_ADDRSTRLEN);
        std::cout << "Connection accepted from instructor's server at " << instructor_ip_str << ":" << ntohs(instructor_addr.sin_port) << std::endl;
        recv(instructor_conn_fd, &instructorBuffer, sizeof(instructorBuffer) - 1, 0);
        close(instructor_conn_fd);
        close(serverFd);

        /* code */
    }

    return 0;
}