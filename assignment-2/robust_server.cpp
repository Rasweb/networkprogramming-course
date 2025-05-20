#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <poll.h>
#define SERVER_PORT 8080
#define MAX_CLIENTS 10
#define IP_TO_LISTEN_TO "127.0.0.1"

// Info - https://thelinuxcode.com/use-poll-system-call-c/

/* poll() info

 int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 - fds – Array of pollfd structures representing the file descriptors to monitor
 - nfds – Number of pollfd structures provided in fds array
 - timeout – Maximum time to wait for an event in milliseconds


 To use poll():
 1. First initialize an array of pollfd structures, one for each fd you want to monitor.
 2. For each pollfd, set the .fd field to the descriptor and .events to the event(s) to check for.
 3. Call poll() passing your pollfd array and a timeout.
 4. When poll() returns, check the .revents field of each pollfd to see which are ready.

*/

int main()
{

    /* socket config - server
    - socket()
    - bind()
    - listen()
    - non blocking?
    */

    /* poll() config
    - poll()
    - std::vector<pollfd> fds;
    - fds.push_back({server_fd, POLLIN, 0});
    */

    /* thread
    - vector of threads
    - create thread pool
      - https://www.geeksforgeeks.org/thread-pool-in-cpp/
      - https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
      - https://dev.to/ish4n10/making-a-thread-pool-in-c-from-scratch-bnm
      - https://medium.com/@lionkor/writing-a-simple-and-effective-thread-pool-in-c-b7675c501da7
    */

    /* poll
    - while(1)
    - loop with fds.size()
      - accept new connection
      - push into fds[] (add new socket to array)
    - else
      - handle client data
      - read data and dispatch to worker thread for processing


    */

    // array of sockets to monitor
    int sockfds[10];

    // fill sockfds array with connected sockets
    struct pollfd pollfds[10];

    // initialize pollfd array
    for (int i = 0; i < 10; i++)
    {
        /* code */
        pollfds[i].fd = sockfds[i];
        pollfds[i].events = POLLIN;
    }

    // wait for data on any socket
    int ready = poll(pollfds, 10, -1);

    for (int i = 0; i < 10; i++)
    {
        /* code */
        if (pollfds[i].revents & POLLIN)
        {
            // Socket i has data, can read without blocking
        }
    }
    return 0;
}