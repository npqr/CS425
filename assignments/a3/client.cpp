#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// Constants
#define SERVER_PORT 12345
#define CLIENT_IP "127.0.0.1"
#define SERVER_IP "127.0.0.1"

// Helper function to send a packet.
void send_packet(int sock, struct sockaddr_in &dest_addr, uint32_t seq, uint32_t ack_seq, bool syn, bool ack) {
    // Allocate enough memory for IP header + TCP header.
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    // IP header pointer
    struct iphdr *ip = (struct iphdr *) packet;
    // TCP header pointer
    struct tcphdr *tcp = (struct tcphdr *) (packet + sizeof(struct iphdr));

    // Fill in the IP header fields
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(packet));
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    // Source is the client
    ip->saddr = inet_addr(CLIENT_IP);
    // Destination is the server
    ip->daddr = inet_addr(SERVER_IP);

    // Fill in the TCP header fields
    // Using a fixed source port for clarity; you may choose any ephemeral port.
    tcp->source = htons(54321); 
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(seq);
    tcp->ack_seq = htonl(ack_seq);
    tcp->doff = 5;  // TCP header size in 32-bit words
    // Set flag bits
    tcp->syn = syn ? 1 : 0;
    tcp->ack = ack ? 1 : 0;
    tcp->fin = 0;
    tcp->rst = 0;
    tcp->psh = 0;
    tcp->window = htons(8192);
    tcp->check = 0; // Leaving checksum as 0 (kernel may fill this in or it may be omitted for the assignment)

    // Send the packet
    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto() failed");
        exit(EXIT_FAILURE);
    } else {
        if(syn && !ack)
            std::cout << "[+] Sent SYN packet" << std::endl;
        else if(syn && ack)
            std::cout << "[+] Sent SYN-ACK packet (unexpected on client)" << std::endl;
        else if(ack)
            std::cout << "[+] Sent final ACK packet" << std::endl;
    }
}

int main() {
    // Create a raw socket for TCP packets.
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Enable IP header inclusion
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // Set up the destination server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // --------------------------
    // Step 1. Send SYN packet.
    // --------------------------
    // As per server code, use sequence number 200 for the SYN
    uint32_t client_syn_seq = 200;
    send_packet(sock, server_addr, client_syn_seq, 0, true, false);

    // --------------------------
    // Step 2. Wait for SYN-ACK response.
    // --------------------------
    //

    // add a timeout to avoid blocking indefinitely
    // this is optional and can be removed if you want to block indefinitely
    struct timeval timeout;
    timeout.tv_sec = 5; // 5 secondU
    timeout.tv_usec = 0;

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }
    

    while(true) {
        char buffer[65536];
        memset(buffer, 0, sizeof(buffer));
        struct sockaddr_in src_addr;
        socklen_t addr_len = sizeof(src_addr);
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&src_addr, &addr_len);

        if (data_size < 0) {
            //perror("recvfrom() failed");
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::cerr << "[-] TIMED OUT" << std::endl;
                exit(EXIT_FAILURE);
            } else {
                perror("recv failed");
                continue;
            }
        }

        // Get the IP header and then the TCP header
        struct iphdr *ip = (struct iphdr *) buffer;
        struct tcphdr *tcp = (struct tcphdr *) (buffer + ip->ihl * 4);

        // Filter for packets coming from the server port
        if (ntohs(tcp->dest) != 54321) {  // Our source port is 54321
            continue;
        }

        // Check if it's the SYN-ACK response: SYN and ACK flags set
        if (tcp->syn == 1 && tcp->ack == 1) {
            uint32_t server_seq = ntohl(tcp->seq);
            uint32_t expected_ack = client_syn_seq + 1; // should be 201
            uint32_t server_ack = ntohl(tcp->ack_seq);
            std::cout << "[+] Received packet: SYN=" << (int)tcp->syn
                      << " ACK=" << (int)tcp->ack
                      << " SEQ=" << server_seq
                      << " ACK_SEQ=" << server_ack << std::endl;

            if (server_seq == 400 && server_ack == expected_ack) {
                std::cout << "[+] SYN-ACK from server recognized." << std::endl;
                break;
            }
        }
    }

    // --------------------------
    // Step 3. Send final ACK packet.
    // --------------------------
    // From the assignment details, the client must now send an ACK packet with:
    //   - Sequence number: 600
    //   - Acknowledgment number: server's sequence (400) + 1 = 401
    uint32_t client_final_seq = 600;
    uint32_t client_final_ack = 401;
    send_packet(sock, server_addr, client_final_seq, client_final_ack, false, true);

    std::cout << "[+] Handshake complete, closing socket." << std::endl;
    close(sock);
    return 0;
}
