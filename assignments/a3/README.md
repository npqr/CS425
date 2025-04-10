# TCP Handshake Assignment

This project implements the client side of a simplified TCP three-way handshake using raw sockets in C++. The goal was to manually construct packet headers for SYN, SYN-ACK, and ACK messages, thereby gaining a deeper understanding of TCP connection establishment.

## Compilation & Running the Code

- It is recommended to run this project on a Linux machine.
- The use of raw sockets typically requires superuser permissions.

### Compiling the Client Code
Run make to compile the code. This will create two executables named `server` and `client`:
```bash
make all
```

### Running the Code
1. **Launch the Server and Client Programs:**
   Execute the compiled server and client programs with superuser permissions: (preferably in separate terminal windows)
   
   ```bash
   sudo ./server
   sudo ./client
   ```

2. **Expected Behavior:**
   - The client first sends a SYN packet with sequence number 200.
   - It waits for a SYN-ACK from the server (with expected server sequence number 400 and acknowledgment number 201).
   - Once the correct SYN-ACK is received, the client sends a final ACK packet with sequence number 600 and acknowledgment number 401.
   - The console output will show logs indicating each step of the handshake.
   
## Design Decisions

- Implemented using raw sockets to allow complete control over TCP packet fields to simulate TCP packets.

- A 5-second timeout is added when waiting for the SYN-ACK packet. This prevents the client from blocking indefinitely if the response is not received.

- We are assuming that both the client and server are running on the same machine. The server is set to listen on port 12345 while the client listens on port 54321.

## Team Members

- Khushi Gupta (220531)
- Nevish Pathe (220757)

## Declaration

We declare that we did not indulge in plagiarism and the work submitted is our own.
