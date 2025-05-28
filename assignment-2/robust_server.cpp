#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <thread>
#include <fcntl.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <chrono>
#include <map>
#define SERVER_PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096
#define IP_TO_LISTEN_TO "127.0.0.1"
#define TIMEOUT 30

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

struct SelectServerState{
  fd_set master_fds;
  fd_set read_fds;
  int max_fd;
  int active_clients = 0;
  struct timeval tv;
  std::map<int, std::string> client_buffer;
  // Timer
  std::map<int, std::chrono::steady_clock::time_point> last_active;
  std::chrono::seconds timeout;
};

void errorCheck(int var, const char *msg);
int socketConfig();
bool setNonBlocking(int sockfd);
void handleIdleDuration(std::map<int, std::chrono::steady_clock::time_point> &last_active, std::chrono::seconds timeout, fd_set *master_fds, int *active_clients);
void handleMaxClients(int active_clients, int server_fd);
void handleIncomingMessages(std::map<int, std::string> &client_buffer, int i, std::map<int, std::chrono::steady_clock::time_point> &last_active, char buffer[], ssize_t bytes_read, ThreadPool *pool);
void selectLoop(int server_fd, ThreadPool *pool);

int main()
{
  ThreadPool newPool;
  newPool.start();
  int socketFd;
  socketFd = socketConfig();
  selectLoop(socketFd, &newPool);
  newPool.stop();
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

bool setNonBlocking(int sockfd)
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

// Goes through a map of clients and last_active time.
// If it a client has passed the timeout close it.
// Removes the file descriptor from the set.
// Decreases the active_clients variable.
// Also removes it from the map.
void handleIdleDuration(
  std::map<int, std::chrono::steady_clock::time_point> &last_active, 
  std::chrono::seconds timeout, 
  fd_set *master_fds, 
  int *active_clients)
{
  auto now = std::chrono::steady_clock::now();
  for (auto it = last_active.begin(); it != last_active.end();)
  {
    if (now - it->second > timeout)
    {
      int sock = it->first;
      std::cout << "Disconnecting idle client" << sock << std::endl;
      close(sock);
      FD_CLR(sock, master_fds);
      if (*active_clients < 0)
      {
        *active_clients = 0;
      }
      (*active_clients)--;
      it = last_active.erase(it);
      std::cout << "Nr of active clients: " << *active_clients << std::endl;
    }
    else
    {
      ++it;
    }
  }
}

// Handles max clients using std::chrono
void handleMaxClients(int active_clients, int server_fd)
{
  std::cout << "Maximum number of clients reached. Refusing connection" << std::endl;
  struct sockaddr_in temp_client_addr;
  socklen_t temp_client_addr_len = sizeof(temp_client_addr);
  int temp = accept(server_fd, (struct sockaddr *)&temp_client_addr, &temp_client_addr_len);
  const char *msg = "Server is full!";
  int sent = send(temp, msg, strlen(msg), 0);
  if (sent < 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      return;
    }
    else
    {
      perror("send failed");
      close(temp);
      return;
    }
  }
}

// Handles incoming messages from client
// Checks if msg contains a newline else it will leave the function
// If a valid msg is recieved the it will use the threadpool and work
void handleIncomingMessages(
  std::map<int, std::string> &client_buffer, 
  int i, 
  std::map<int, std::chrono::steady_clock::time_point> &last_active, 
  char buffer[], 
  ssize_t bytes_read, 
  ThreadPool *pool)
{
  size_t pos;
  //std::string::npos means not found
  while ((pos = client_buffer[i].find('\n')) != std::string::npos)
  {
    std::string message = client_buffer[i].substr(0, pos + 1);
    client_buffer[i].erase(0, pos + 1);

    last_active[i] = std::chrono::steady_clock::now();
    buffer[bytes_read] = '\0';
    std::cout << "Received from socket: " << buffer << std::endl;

    pool->queueJob([i, buffer, bytes_read](){
      std::cout << "Thread id:" << std::this_thread::get_id() << std::endl;
      ssize_t total_sent = 0;
      while (total_sent < bytes_read){
        int sent = send(i, buffer + total_sent, bytes_read - total_sent, 0); 
        if (sent == -1) {
          std::cerr << "Failed to send data to client." << std::endl;
          break;
        }
        if (sent < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
          else {
            perror("send failed");
            close(i);
            break;
          }
        } 
        total_sent += sent;
      }
      if(total_sent == bytes_read){
        std::cout << "Sent " << total_sent << " bytes to client." << std::endl;
      } 
    });
  }
}

// Configures and manages a server using the select() system call to handle multiple client connections.
// Initializes file descriptor(fd) sets for monitoring multiple sockets.
// New connections: Accepting them and adding their sockets to monitored set.
// Existing connections, it reads incoming data aznd echoes it back to the client.
void selectLoop(int server_fd, ThreadPool *pool)
{

  SelectServerState state;

  FD_ZERO(&state.master_fds);
  FD_ZERO(&state.read_fds);
  FD_SET(server_fd, &state.master_fds);

  state.max_fd = server_fd;
  state.active_clients = 0;
  state.tv.tv_sec = TIMEOUT;
  state.tv.tv_usec = 0;
  state.timeout = std::chrono::seconds(TIMEOUT);

  std::cout << "Server listening on port: " << SERVER_PORT << " (using select)..."
            << std::endl;

  while (true)
  {
    state.read_fds = state.master_fds;

    int activity = select(state.max_fd + 1, &state.read_fds, NULL, NULL, &state.tv);
    errorCheck(activity, "select error");

    if (activity < 0)
    {
      perror("Select error");
      break;
    }

    handleIdleDuration(state.last_active, state.timeout, &state.master_fds, &state.active_clients);

    if (activity == 0)
    {
      continue;
    }

    if (FD_ISSET(server_fd, &state.read_fds))
    {
      if (state.active_clients >= MAX_CLIENTS)
      {
        handleMaxClients(state.active_clients, server_fd);
      }
      else
      {
        struct sockaddr_in temp_client_addr;
        socklen_t temp_client_addr_len = sizeof(temp_client_addr);

        int new_client_sock =
            accept(server_fd, (struct sockaddr *)&temp_client_addr,
                   &temp_client_addr_len);
        state.last_active[new_client_sock] = std::chrono::steady_clock::now();

        int optErr;
        optErr = setsockopt(new_client_sock, SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&state.tv, sizeof(struct timeval));
        if (optErr < 0)
        {
          std::cout << "Error";
        }
        optErr = setsockopt(new_client_sock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&state.tv, sizeof(struct timeval));
        if (optErr < 0)
        {
          std::cout << "Error";
        }

        if (new_client_sock < 0)
        {
          if (errno == EAGAIN || errno == EWOULDBLOCK)
          {
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
        if (!setNonBlocking(new_client_sock))
        {
          close(new_client_sock);
          continue;
        }

        FD_SET(new_client_sock, &state.master_fds);
        if (new_client_sock > state.max_fd)
        {
          state.max_fd = new_client_sock;
        }
        state.active_clients++;
        std::cout << "Nr of active clients: " << state.active_clients << std::endl;
      }
    }

    for (int i = 0; i <= state.max_fd; i++)
    {
      if (i == server_fd)
      {
        continue;
      }
      if (FD_ISSET(i, &state.read_fds))
      {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = recv(i, buffer, BUFFER_SIZE - 1, 0);
        state.client_buffer[i].append(buffer, bytes_read);
        if (bytes_read == BUFFER_SIZE - 1 && strchr(buffer, '\n') == nullptr)
        {
          std::cout << "Message too long for thread: " << std::this_thread::get_id() << std::endl;
          const char *err = "Error: message too large\n";
          send(i, err, strlen(err), 0);
          state.client_buffer[i].clear();
          continue;
        }
        if (bytes_read > 0)
        {
          handleIncomingMessages(state.client_buffer, i, state.last_active, buffer, bytes_read, pool);
        }
        if (bytes_read == 0)
        {
          {
            state.last_active.erase(i);
            if (state.max_fd == i)
            {
              for (int i = 0; i <= state.max_fd; i++)
              {
                if (i > state.max_fd)
                {
                  state.max_fd = i;
                }
              }
            }
            std::cout << "Client on socket disconnected." << std::endl;
            state.client_buffer[i].clear();
            close(i);
            FD_CLR(i, &state.master_fds);
            state.active_clients--;
          }
        }
        if (bytes_read < 0)
        {
          if (errno != EAGAIN || errno != EWOULDBLOCK)
          {
            state.last_active.erase(i);
            perror("recv failed");
            std::cerr << "Error on recv" << std::endl;
            close(i);
            FD_CLR(i, &state.master_fds);
            state.active_clients--;
          }
        }
      }
    }
  }
}