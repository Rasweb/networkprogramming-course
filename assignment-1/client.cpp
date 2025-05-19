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

int createSocket(int clientFd, const char *errMsg, const char *connectErrMsg);
void errorCheck(int var, const char *msg);
void sendAndReceiveFunc(char numb, int sendReturn, int clientfd, const char *sendMsg, const char *connectMsg, bool sendUrgent, const char *recvMsg, bool urgentNoData);

int main()
{
    int clientSocket;
    int socketSendReturn;
    const char *socketSendMsg1 = "NORMAL_DATA:Hello";
    const char *socketSendMsg2 = "SEND_URGENT_REQUEST";
    const char *socketSendMsg3 = "SEND_URGENT_REQUEST";
    const char *socketSendMsg4 = "TEST_GARBAGE";
    const char *socketConnectMsg = "Socket connection, error type";
    const char *socketCreationErr = "Socket creation, error type";
    const char *socketRecvError = "Socket recv, error type";

    clientSocket = createSocket(clientSocket, socketCreationErr, socketConnectMsg);
    sendAndReceiveFunc('1', socketSendReturn, clientSocket, socketSendMsg1, socketConnectMsg, false, socketRecvError, false);
    std::cout << std::endl;

    clientSocket = createSocket(clientSocket, socketCreationErr, socketConnectMsg);
    sendAndReceiveFunc('2', socketSendReturn, clientSocket, socketSendMsg2, socketConnectMsg, true, socketRecvError, false);
    std::cout << std::endl;

    clientSocket = createSocket(clientSocket, socketCreationErr, socketConnectMsg);
    sendAndReceiveFunc('3', socketSendReturn, clientSocket, socketSendMsg3, socketConnectMsg, false, socketRecvError, true);
    std::cout << std::endl;

    clientSocket = createSocket(clientSocket, socketCreationErr, socketConnectMsg);
    sendAndReceiveFunc('4', socketSendReturn, clientSocket, socketSendMsg4, socketConnectMsg, false, socketRecvError, false);

    return 0;
}

int createSocket(int clientFd, const char *errMsg, const char *connectErrMsg)
{
    int socketConnectReturn;
    clientFd = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(clientFd, errMsg);

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, IP_TO_LISTEN_TO, &serverAddress.sin_addr) <= 0)
    {
        std::cerr << "Invalid address or not supported" << std::endl;
        close(clientFd);
        return 0;
    }

    socketConnectReturn = connect(clientFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    errorCheck(socketConnectReturn, connectErrMsg);

    return clientFd;
}

void errorCheck(int var, const char *msg)
{
    if (var < 0)
    {
        std::cerr << "Something went wrong with: ";
        perror(msg);
    }
}

void sendAndReceiveFunc(char numb, int sendReturn, int clientfd, const char *sendMsg, const char *connectMsg, bool sendUrgent, const char *recvMsg, bool urgentNoData)
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
    if (urgentNoData)
    {
    }
    char buffer[1024];
    int reciveBytes = recv(clientfd, buffer, sizeof(buffer), 0);
    errorCheck(reciveBytes, recvMsg);

    if (reciveBytes > 0)
    {
        buffer[reciveBytes] = '\0';
        std::cout << "Recived: " << buffer << std::endl;
    }
    close(clientfd);
}