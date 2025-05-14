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

void errorCheck(int var, std::string msg);
void sendAndReceiveFunc(char numb, int sendReturn, int clientfd, const char *sendMsg, std::string connectMsg, bool sendUrgent);

int main()
{
    int clientSocket;
    int socketConnectReturn;
    int socketSendReturn;
    std::string dataToSend = "NORMAL_DATA:Hello";
    std::string socketConnectMsg = "Socket connect msg";
    const char *socketSendMsg1 = "NORMAL_DATA:Hello";
    const char *socketSendMsg2 = "SEND_URGENT_REQUEST";

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::string message_to_send = dataToSend;

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
    errorCheck(socketConnectReturn, socketConnectMsg);

    sendAndReceiveFunc('1', socketSendReturn, clientSocket, socketSendMsg1, socketConnectMsg, false);
    std::cout << std::endl;
    sendAndReceiveFunc('2', socketSendReturn, clientSocket, socketSendMsg2, socketConnectMsg, true);

    close(clientSocket);

    return 0;
}

void errorCheck(int var, std::string msg)
{
    if (var < 0)
    {
        std::cerr << msg << ": " << errno << std::endl;
    }
}

void sendAndReceiveFunc(char numb, int sendReturn, int clientfd, const char *sendMsg, std::string connectMsg, bool sendUrgent)
{
    std::cout << "Scenario " << numb << std::endl;
    sendReturn = send(clientfd, sendMsg, strlen(sendMsg), 0);
    errorCheck(sendReturn, connectMsg);
    std::cout << "Sent: " << sendMsg << std::endl;

    if (sendUrgent)
    {
        sendReturn = send(clientfd, "U", 1, MSG_OOB);
        errorCheck(sendReturn, "Send one byte error");
    }
    char buffer[1024];
    int reciveBytes = recv(clientfd, buffer, sizeof(buffer), 0);
    errorCheck(reciveBytes, "Recieve error");

    if (reciveBytes > 0)
    {
        buffer[reciveBytes] = '\0';
        std::cout << "Recived: " << buffer << std::endl;
    }
}