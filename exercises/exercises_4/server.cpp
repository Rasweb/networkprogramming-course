#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

bool set_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);

    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return false;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
        return false;
    }

    return true;
}

int main()
{
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server socket created." << std::endl;

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt SO_REUSEADDR failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (!set_non_blocking(server_fd))
    {
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Listening socket set to non-blocking." << std::endl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket bound to port " << PORT << std::endl;

    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    fd_set master_fds, read_fds;
    int max_fd;

    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);

    FD_SET(server_fd, &master_fds); // master_fds = 0000 0100
    max_fd = server_fd;

    std::cout << "Server listening on port: " << PORT << " (using select)..."
              << std::endl;

    while (true)
    {
        read_fds = master_fds;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR)
        {
            perror("Select error");
            break;
        }

        if (activity == 0)
        {
            // Shouldn't happen when TIMEOUT == NULL!
            continue;
        }

        if (FD_ISSET(server_fd, &read_fds))
        {
            // Someone has connected!
            struct sockaddr_in temp_client_addr;
            socklen_t temp_client_addr_len = sizeof(temp_client_addr);

            int new_client_sock =
                accept(server_fd, (struct sockaddr *)&temp_client_addr,
                       &temp_client_addr_len);

            if (new_client_sock < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // This would be suprising, no? Why should this not happen?
                    continue;
                }
                perror("Error accepting socket!");
                continue;
            }
            char client_ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &temp_client_addr.sin_addr, client_ip_str,
                      INET_ADDRSTRLEN);
            std::cout << "Accepted connection from " << client_ip_str << ":"
                      << ntohs(temp_client_addr.sin_port) << std::endl;
            if (!set_non_blocking(new_client_sock))
            {
                close(new_client_sock);
                continue;
            }

            FD_SET(new_client_sock, &master_fds);
            if (new_client_sock > max_fd)
            {
                max_fd = new_client_sock;
            }
        }

        for (int i = 0; i < max_fd; i++)
        {
            if (i == server_fd)
            {
                continue;
            }
            if (FD_ISSET(i, &read_fds))
            {
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));
                ssize_t bytes_read = recv(i, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read > 0)
                {
                    buffer[bytes_read] = '\0';
                    std::cout << "Received from socket: " << buffer << std::endl;
                    int senderr = send(i, buffer, bytes_read, 0);
                    if (senderr < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            std::cout << "Send on socket would block, data not sent this time." << std::endl;
                        }
                        else
                        {
                            perror("send failed");
                            close(i);
                            FD_CLR(i, &master_fds);
                        }
                    }
                }
                if (bytes_read == 0)
                {
                    {
                        std::cout << "Client on socket disconnected." << std::endl;
                        close(i);
                        FD_CLR(i, &master_fds);
                    }
                }
                if (bytes_read < 0)
                {
                    if (errno != EAGAIN || errno != EWOULDBLOCK)
                    {
                        perror("recv failed");
                        std::cerr << "Error on recv" << std::endl;
                        close(i);
                        FD_CLR(i, &master_fds);
                    }
                }
            }
        }
    }
    return 0;
}