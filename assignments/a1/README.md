# README

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

## Implementation

### High-Level Idea of Important Functions
- **`load_credentials`**: Loads user credentials from `users.txt`.
- **`send_message`**: Sends a message to a specific client.
- **`create_group`**: Creates a new group and adds the client to it.
- **`join_group`**: Adds a client to an existing group.
- **`leave_group`**: Removes a client from a group.
- **`broadcast_message`**: Sends a message to all connected clients.
- **`private_message`**: Sends a private message to a specific user.
- **`group_message`**: Sends a message to all members of a group.
- **`list_commands`**: Lists all available commands.
- **`list_groups`**: Lists all groups a user is a member of.
- **`list_members`**: Lists all members of a group.
- **`handle_client`**: Handles client connection and processes commands.
- **`sigint_handler`**: Handles graceful shutdown of the server.

### Code Flow

1. **Server Initialization**:
   - Load user credentials.
   - Create a server socket and bind it to a port.
   - Listen for incoming connections.
2. **Client Connection**:
   - Accept incoming client connections.
   - Create a new thread to handle each client.
3. **Client Handling**:
   - Authenticate the client.
   - Process client commands (broadcast, private message, group message, etc.).
   - Handle client disconnection.
4. **Graceful Shutdown**:
   - Handle `SIGINT` signal to shut down the server gracefully.

## Testing

### Correctness Testing
- Used Google Test framework for unit testing various components of the server.
- Tested basic server functionality by connecting multiple clients and verifying message delivery.
- Verified user authentication by testing with valid and invalid credentials.
- Tested group management features by creating, joining, and leaving groups.
- Verified message broadcasting and private messaging.

### Stress Testing
- Used `xargs` to send multiple requests simultaneously to test server concurrency and performance.

## Challenges

### Design Challenges
- Initially considered using processes for each connection but opted for threads due to lower overhead and easier shared resource management.
- Ensured thread-safe access to shared resources using `std::mutex` and `std::lock_guard`.

### Implementation Challenges
- Faced issues with thread synchronization, resolved by carefully locking shared resources.
- Debugged issues with client disconnection handling and ensured proper cleanup of resources.

## Restrictions

### Server Limitations
- **Maximum Clients**: Limited by system resources and thread handling capacity.
- **Maximum Groups**: Limited by system memory.
- **Maximum Group Members**: Limited by system memory.
- **Message Size**: Limited to `BUFFER_SIZE` (1024 bytes).

## Individual Contributions

### Contribution of Member 1
- Designed and implemented the server architecture.
- Handled user authentication and message broadcasting features.

### Contribution of Member 2
- Implemented group management features.
- Handled client connection and disconnection logic.

### Contribution of Member 3
- Conducted testing and debugging.
- Prepared the README and documentation.

## Sources
- Beej's Guide to Network Programming: https://beej.us/guide/bgnet/
- C++ Reference: https://en.cppreference.com/
- Google Test Documentation: https://github.com/google/googletest

## Declaration
We declare that we did not indulge in plagiarism and the work submitted is our own.

## Feedback
- The assignment was challenging and provided a good learning experience in network programming and multithreading.
- Suggest providing more examples and edge cases for testing.