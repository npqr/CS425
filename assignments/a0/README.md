# HTTP Server Guide

## Directory Structure

(after compiling and running the server)

```
a0/
├── Makefile
├── http_server.cpp
├── http_server
├── http_server.o
├── static/
│   ├── 404.html
│   ├── index.html
│   ├── myfiles/
│   │   └── index.html
│   ├── script.js
│   ├── secrets/
│   │   ├── .env
│   │   ├── robots.txt
│   │   ├── secret.txt
│   │   └── stuff.json
│   ├── styles.css
└─── server.log
```

## How to Run the Server

### Prerequisites

- Ensure you have `g++` installed on your system. (compiled with C++20)
- Ensure you have `make` installed on your system.

### Steps

1. **Build the Server:**

    In the project directory, use the `make` command to compile the server:

   ```sh
   make
   ```

   This will generate an executable named `http_server`
2. **Run the Server:**

   Execute the server with the desired port and static directory:

   ```sh
   ./http_server <port> <static_directory>
   ```

   For example, to run the server on port 8080 and serve files from the `static` directory:

   ```sh
   ./http_server 8080 ./static
   ```

3. **Access the Server:**

   Open your web browser and navigate to `http://127.0.0.1:<port>/`.

### Cleaning Up

To clean up the build files and the log file, run:

```sh
make clean
```

This will remove the compiled object files, the executable, and `server.log`

## Features

Supports the required features for starting the server, handling GET requests, and serving static files. The server logs requests to `server.log` and handles errors gracefully by returning a 404 response when a file is not found.

Additional features include:

- **Directory Listing:** If a directory is requested, the server will display a list of files and subdirectories. For example, accessing the `secrets` directory will show the contents of that directory, while accessing `myfiles` will serve the `index.html` present in that directory.

- **MIME Type Detection:** The server detects the MIME type of files based on their extension and sends the appropriate `Content-Type` header.

- **Graceful Shutdown:** The server can be gracefully shut down by sending a `SIGINT` signal (Ctrl+C).

- **Concurrency:** The server uses multiple threads to handle incoming requests concurrently.

## Testing

You can test the server by accessing different URLs and files. For concurrency testing, we can use `xargs` to send multiple requests simultaneously:

```sh
seq 10 | xargs -n 1 -P 10 curl -s http://127.0.0.1:8080/cat.png
```

<!-- Author: npqr       -->
<!-- 22:22 12-01-2025   -->
