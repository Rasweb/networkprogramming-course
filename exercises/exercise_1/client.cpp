#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024
int main()
{
    const char *msg_after = "wakwaka";
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <=
        0)
    {
        std::cerr << "Invalid address or address not supported" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Connecting to server..." << std::endl;
    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        return 1;
    }

    char buffer[BUFFER_SIZE] = {0};
    int valread = read(sock, buffer, BUFFER_SIZE);

    std::cout << "Message from server: " << buffer << std::endl;

    send(sock, msg_after, strlen(msg_after), 0);
    std::cout << "Connection message sent" << std::endl;

    close(sock);
    return 0;
}