#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#define SERVER_PORT 8080
#define MAX_CLIENTS 10
#define IP_TO_LISTEN_TO "127.0.0.1"

/* Usage
- start() to initialize
- queueJob() to add tasks
- call stop() when done
*/
class ThreadPool
{
private:
  // Thread pool conditon if it should stop with new jobs and terminate
  bool should_terminate = false;
  // protect the data from concurent access
  std::mutex mutex;
  // Used to block threads until there are jobs available or if pool is set to terminate
  std::condition_variable cv;
  // vector of threads
  std::vector<std::thread> threads;
  // holds functions to be executed by the threads
  std::queue<std::function<void()>> jobs;

  /* Waiting for task queue to open up
  - Runs in each thread
  - Waits for jobs to be available
  - uses unique_lock to acquire mutex
  - if should_terminate is true it exists loop and the thread is done
  - if function(job) available it retrieves the job from the queue and runs it.
  */
  void ThreadLoop()
  {
    while (true)
    {
      std::function<void()> job;
      {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this]
                { return !jobs.empty() || should_terminate; });
        if (should_terminate)
        {
          return;
        }
        job = jobs.front();
        jobs.pop();
      }
      job();
    }
  }

public:
  /*  thread waiting for new tasks
  - initializes the thread pool by creating max threads that the system can handle
  - Each thread runs ThreadLoop()
  */
  void start()
  {
    // Max threads the system supports
    const uint32_t num_threads = std::thread::hardware_concurrency();
    for (uint32_t ii = 0; ii < num_threads; ++ii)
    {
      // removes last element
      threads.emplace_back(std::thread(&ThreadPool::ThreadLoop, this));
    }
  }

  /* To use: thread_pool->queueJob([{}]);
  - Add new jobs(functions) to the pool
  - Locks mutex, pushes job onto queue
  - Notifies wating thread that a job is available
  */
  void queueJob(const std::function<void()> &job)
  {
    // scoping of the unique_lock
    // releases lock after code leaves scope
    {
      // prevents multiple usese simultaneously
      std::unique_lock<std::mutex> lock(mutex);
      jobs.push(job);
    }
    cv.notify_one();
  }

  /* Determines if the thread pool is currently processing any tasks
  - Checks if there are any jobs(functions) in queue
  - Locks mutex, to safely access the queue
  - Returns true if there are jobs
  - Returns False if there are no jobs
  */
  bool busy()
  {
    bool poolbusy;
    {
      std::unique_lock<std::mutex> lock(mutex);
      poolbusy = !jobs.empty();
    }
    return poolbusy;
  }

  /*
  - Tells the thread pool to stop accepting new jobs and to terminate
  - Joins each thread (Waits for them to finnish)
  - Clears vector of threads
  */
  void stop()
  {
    {
      std::unique_lock<std::mutex> lock(mutex);
      should_terminate = true;
    }
    cv.notify_all();
    for (std::thread &active_thread : threads)
    {
      active_thread.join();
    }
    threads.clear();
  }
};

void errorCheck(int var, const char *msg);
int socketConfig();
void pollConf();

// Implement comments for docs
int main()
{

  // Netcat for clients
  int socketFd;
  socketFd = socketConfig();

  // 1
  /*threadpool class
    - constructor
    - destructor
    - enqueue
    - thread
    - mutex
  */
  /* worker threads
  - Create a pool of worker threads in the ThreadPool constructor.
  - Each worker thread should continuously check for tasks in a task queue.
  - Use a std::queue to manage tasks and a std::mutex to protect access to the queue.
  */
  /* Task queue
  Task Queue:
  - Implement a synchronized task queue using a std::mutex and std::condition_variable.
  - The enqueue method should add tasks to the queue and notify a worker thread to process the task.
  */
  /* Task processing
  - Worker threads should pick up tasks from the queue, process them, and send responses back to clients.
  */

  // 2
  /* I/O multiplexing poll()
  - poll() to monitor listening socket and all connected clients
  - pollfd array to keep track of file descriptors
 */
  /* new connections
  -When poll() indicates that the listening socket is ready for reading, accept the new connection using accept().
  - Add the new client socket to the pollfd array for monitoring.
  */
  /* Client requests
  - When poll() indicates that a client socket is ready for reading, read the incoming data from the client.
- Dispatch the request to a worker thread from the thread pool for processing.
  */
  /* responses
- After a worker thread processes the request, it should send the response back to the client.
 -Use poll() to monitor the client socket for writability and send the response.
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

  printf("Hejehje");

  return 0;
}

void errorCheck(int var, const char *msg)
{
  if (var < 0)
  {
    std::cerr << "Something went wrong with: ";
    perror(msg);
    close(var);
  }
}

int socketConfig()
{
  int socketFd;
  int optReturn;
  int bindReturn;
  int listenReturn;
  struct sockaddr_in serverAdress;
  const char *socketCreateMsg = "Socket creation, error type";
  const char *socketOptMsg = "Socket options, error type";
  const char *socketBindMsg = "Socket bind, error type";
  const char *socketListenMsg = "Socket listen, error type";

  socketFd = socket(AF_INET, SOCK_STREAM, 0);
  errorCheck(socketFd, socketCreateMsg);

  int flags = fcntl(socketFd, F_GETFL, 0);
  fcntl(socketFd, F_SETFL, flags | O_NONBLOCK);

  int opt = 1;

  optReturn = setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  errorCheck(optReturn, socketOptMsg);

  memset(&serverAdress, 0, sizeof(serverAdress));
  serverAdress.sin_family = AF_INET;
  serverAdress.sin_addr.s_addr = inet_addr(IP_TO_LISTEN_TO);
  serverAdress.sin_port = htons(SERVER_PORT);

  bindReturn = bind(socketFd, (struct sockaddr *)&serverAdress, sizeof(serverAdress));
  errorCheck(bindReturn, socketBindMsg);

  listenReturn = listen(socketFd, 5);
  errorCheck(listenReturn, socketListenMsg);

  return socketFd;
}

void pollConf()
{
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

  std::vector<pollfd> fds;
  /* poll() config
  - poll()
  - std::vector<pollfd> fds;
  - fds.push_back({server_fd, POLLIN, 0});
  */

  // // array of sockets to monitor
  // int sockfds[10];

  // // fill sockfds array with connected sockets
  // struct pollfd pollfds[10];

  // // initialize pollfd array
  // for (int i = 0; i < 10; i++)
  // {
  //   /* code */
  //   pollfds[i].fd = sockfds[i];
  //   pollfds[i].events = POLLIN;
  // }

  // // wait for data on any socket
  // int ready = poll(pollfds, 10, -1);

  // for (int i = 0; i < 10; i++)
  // {
  //   /* code */
  //   if (pollfds[i].revents & POLLIN)
  //   {
  //     // Socket i has data, can read without blocking
  //   }
  // }
}