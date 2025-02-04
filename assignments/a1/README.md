# CS425 A1: Chat Server with Groups and Private Messages

## Directory Structure

(after running `make` in the root directory, and `make` in the `tests` directory)

```
a1/
├── README.md
├── Makefile
├── server_grp.cpp
├── client_grp.cpp
├── server_grp.h
├── server_grp.o
├── server_grp
├── client_grp
├── users.txt
└── tests/
    ├── README.md
    ├── Makefile
    ├── googletest/
    ├── server_grp_test.cpp
    ├── server_grp_test.h
    ├── server_grp_test.o
    ├── test_log (output of unit test results)
    ├── users.txt.bak (backup of users.txt)
    ├── build_gtest.sh
    ├── make_dummy_users.sh
    └── client.sh
```

## How to Run

### Prerequisites

- Ensure you have `g++` installed on your system. (compiled with C++20)
- Ensure you have `make` installed on your system.

For testing:
- Ensure you have googletest installed on your system.
- Ensure you have `nc` installed on your system.
- Ensure you have `xargs` installed on your system.

### Running

After running `make` in the root directory, run `./server_grp` and `./client_grp` in separate terminals to start the server and client, respectively.

## Features

### Implemented Features
- **Basic Server Functionality**: 
  - TCP-based server that listens on a specific port (`12345`).
  - Accepts multiple concurrent client connections.
  - Maintains a list of connected clients with their usernames.
- **User Authentication**:
  - Stores usernames and passwords in `users.txt`.
  - Prompts users to enter their username and password upon connection.
  - Disconnects clients that fail authentication.
- **Messaging Features**:
  - Broadcast messages to all connected clients using `/broadcast <message>`.
  - Send private messages to a specific user using `/msg <username> <message>`.
  - Send messages to all group members using `/group_msg <group_name> <message>`.
- **Group Management**:
  - Create a new group using `/create_group <group_name>`.
  - Join an existing group using `/join_group <group_name>`.
  - Leave a group using `/leave_group <group_name>`.
  - List all groups a user is a member of using `/list_groups`.
  - List all members of a group using `/list_members <group_name>`.
- **Concurrency**:
  - Uses multiple threads to handle incoming requests concurrently.
- **Graceful Shutdown**:
  - Server can be gracefully shut down by sending a `SIGINT` signal (Ctrl+C).

### Not Implemented Features
- Persistent storage of messages.
- Offline message delivery.

## Design Decisions

### Multithreaded Server
- Each client connection is handled by a separate thread.
- Allows multiple clients to connect concurrently and communicate with the server.

### Persistent Group Memory
- As long as the server is running, it will keep track of all the groups and the members of each group. 
- This allows for easy message broadcasting to all members of a group and ensures that group membership is retained even if a client disconnects.
- BUT if the server is restarted, all information about clients and groups is lost.

### Synchronous I/O
- Uses the `select()` [(ref)](https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html#select) system call to handle multiple client connections.
- Ensures that the server can handle multiple clients without blocking on I/O operations.

### Naming Constraints
- All usernames and group names are case-sensitive alphanumeric strings.
- Group names cannot contain spaces to simplify parsing of commands.
- Usernames cannot contain spaces to simplify authentication and message routing.

## Implementation

### Global Variables and Data Structures

   - `clients_mutex`: A mutex to ensure thread-safe access to shared resources.
   - `clients`: A map that associates client sockets with usernames.
   - `groups`: A map that associates group names with sets of client sockets.
   - `user_credentials`: A map that stores username-password pairs for authentication.
   - `users`: A map that associates usernames with sets of group names.

### Utility Functions

   - `trim`: Trims leading and trailing whitespaces from a string.
   - `load_credentials`: Loads user credentials from a file named `users.txt`.

### Group Management Functions

   - `create_group`: Creates a new group and adds the client to it.
   - `join_group`: Adds a client to an existing group.
   - `leave_group`: Removes a client from a group.
   - `broadcast_message`: Broadcasts a message to all connected clients.
   - `private_message`: Sends a private message to a specific user.
   - `group_message`: Sends a message to all members of a group.
   - `list_commands`: Lists all available commands.
   - `list_groups`: Lists all groups a user is a member of.
   - `list_members`: Lists all members of a group.
   - `grpcmd`: A map that associates group commands with their corresponding handler functions.

## Code Flow

1. **Initialization**:
   - The server initializes global variables and data structures.
   - The `load_credentials()` function loads user credentials from `users.txt`.

2. **Server Setup**:
   - The `main()` function sets up the `SIGINT` signal handler.
   - The server socket is created, bound to a port, and set to listen for incoming connections.

3. **Client Connection Handling**:
   - The server enters a loop where it uses `select()` to monitor the server socket for incoming connections.
   - When a new client connection is detected, the server accepts the connection and creates a new thread to handle the client using the `handle_client()` function.

4. **Client Authentication and Command Handling**:
   - The `handle_client()` function authenticates the client by prompting for a username and password.
   - The client is added to the list of connected clients.
   - The server enters a loop where it listens for commands from the client and processes them accordingly (e.g., broadcasting messages, sending private messages, group messaging, listing commands, groups, and members).
   - Upon client disconnection, the client is removed from the list of connected clients and groups.

5. **Graceful Shutdown**:
   - When the server receives a `SIGINT` signal, the `sigint_handler()` function sets the `running` flag to `false`.
   - The `main()` function exits the loop, broadcasts a shutdown message to all clients, and closes the server socket.

## Testing

Testing was carried on an Arch Linux machine and a macOS machine.

### Correctness Testing
- Used [Google Test framework](https://google.github.io/googletest/) for unit testing various components of the server.
- Tested basic server functionality by connecting multiple clients and verifying message delivery.
- Tested the correctness of authentication, message broadcasting, group management, and private messaging features.
- Manually tested edge cases like invalid commands, incorrect group names, and invalid usernames and invalid situations like sending messages to non-existent users, leaving non-existent groups, etc.

### Stress Testing
- Created 50 dummy clients and simulated their connections using `nc` and `xargs` to send multiple requests simultaneously to test server concurrency and performance.
- Additionally, manually tested the server with 10+ clients connected concurrently. 

## Challenges

### Design Challenges
- Initially, the data structure for storing group information handled clients by their socket descriptors and not their usernames.
- Resolved by storing the mapping of usernames and list of group names in a separate data structure.
- Ensured thread-safe access to shared resources using `std::mutex` and `std::lock_guard`.

### Implementation Challenges
- Encountered blocking issues, resolved by using `select()` to handle multiple client connections.
- Faced issues with thread synchronization, resolved by carefully locking shared resources.
- Debugged issues with client disconnection handling and ensured proper cleanup of resources.

## Restrictions

### Server Limitations
- **Maximum Clients**: Limited by system resources and thread handling capacity.
- **Maximum Groups**: Limited by system memory.
- **Maximum Group Members**: Limited by system memory.
- **Message Size**: Limited to `BUFFER_SIZE` (1024 bytes), can be increased if required.

## Individual Contributions

### Contribution of Member 1 (Khushi Gupta, 220531)
- Designed and implemented the server architecture.
- Implemented the core server functionality.
- Handled authentication, message broadcasting and group management.

### Contribution of Member 2 (Nevish Pathe, 220757)
- Handled command parsing, concurrency and thread management.
- Conducted testing and debugging.
- Prepared the README.

## Sources
- Beej's Guide to Network Programming: https://beej.us/guide/bgnet/
- C++ Reference: https://en.cppreference.com/
- Google Test Documentation: https://github.com/google/googletest

## Declaration
We declare that we did not indulge in plagiarism and the work submitted is our own.

## Feedback
- The assignment was a nice exercise in socket programming and multithreading.
- The project could be extended to include more advanced features like offline messaging and persistent storage.
