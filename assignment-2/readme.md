<h1 style="text-align:center">Table of contents</h1>
<ul>
    <li><a href="#overview">Overview</a>
    </li>
    <li><a href="#architecture">Server Architecture</a>
    </li>
    <li><a href="#protocol">Application Protocol<a>
    </li>
    <li><a href="#robustness">Robustness Requirements</a>
    </li>
    <li><a href="#build-process">Build Process</a>
    </li>
    <li><a href="#built-with">Built With</a>
    </li>
    <li><a href="#limitations">Known Limitations</a>
    </li>
    <li><a href="#in-action">In Action</a></li>
</ul>

<h1 style="text-align:center">Overview</h1>
<h3>Assignment 2 - Robust Server</h3>
<p>
    This project implements a robust TCP server using the <b>select()</b> system call for I/O multiplexing and a <b>thread pool</b> for concurrent request handling. The server is designed to handle multiple clients efficiently and robustly, echoing back any data received.
</p>

<h2 id="architecture">Server Architecture</h2>
<ul>
    <li>
        <b>I/O Multiplexing:</b> The server uses <code>select()</code> to monitor multiple client sockets for incoming data or new connections. This allows the server to efficiently handle multiple clients without dedicating a thread to each connection.
    </li>
    <li>
        <b>Thread Pool Model:</b> Instead of spawning a new thread for each request (thread-per-request), the server uses a thread pool. When data is received from a client, the main thread enqueues a job to the thread pool, which then processes the request (in this case, echoing the data back). This approach reduces thread creation overhead and improves scalability.
    </li>
    <li>
        <b>Interaction:</b> <code>select()</code> detects ready sockets, and the main thread reads incoming data. The actual processing and response (echo) are offloaded to the thread pool, allowing the main loop to remain responsive to new events.
    </li>
</ul>

<h2 id="protocol">Application Protocol</h2>
<ul>
    <li>
        <b>Echo Protocol:</b> The server implements a simple echo protocol. Any data sent by a client is echoed back verbatim. There are no special commands or message framing; the server simply sends back whatever it receives.
    </li>
</ul>


<h2 id="robustness">Robustness Requirements</h2>
<ul>
    <li>
        <b>Partial Reads/Writes:</b> The server handles partial reads and writes by buffering data and looping until the entire message is received or sent.
    </li>
    <li>
        <b>Non-blocking I/O:</b> All sockets are set to non-blocking mode to prevent the server from hanging on slow or unresponsive clients.
    </li>
    <li>
        <b>Idle Client Timeout:</b> Clients that remain idle for more than 30 seconds are automatically disconnected to free up resources.
    </li>
    <li>
        <b>Resource Limits:</b> The server enforces a maximum number of concurrent clients (<code>MAX_CLIENTS</code>). New connections are refused with a message if the limit is reached.
    </li>
    <li>
        <b>Error Handling:</b> All system calls are checked for errors, and appropriate cleanup is performed on failure.
    </li>
</ul>

<h2>Build process</h2>

```cmake
cd build

// when changes to the cmake
cmake ..

// When changes to cpp file
make

./robust
```
<h3 id="built-with">Built With</h3>

- C++
- POSIX sockets
- select()
- Thread pool (std::thread, std::mutex, std::condition_variable)

<h3 id="limitations">Known Limitations / Assumptions</h3>
<ul>
    <li>
        The server only listens on <code>127.0.0.1:8080</code> (localhost).
    </li>
    <li>
        The protocol is a simple echo; no message framing or command parsing.
    </li>
    <li>
        The maximum number of clients is fixed at compile time.
    </li>
    <li>
        No SSL/TLS support.
    </li>
    <li>
        The server does not persist data between restarts.
    </li>
</ul>

<h3 id="in-action">In Action</h3>
<p>
    You can test the server using <code>nc 127.0.0.1 8080</code> from multiple terminals. Type messages and observe them being echoed back.
</p>
<p>
    You can also test the server using my script: <code> ./s_script.sh <code>.
    It will connect 10 clients and send e.g: "Test 1"
</p>