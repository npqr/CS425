#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define SERVER_PORT 12345
#define CLIENT_PORT 54321
#define CLIENT_IP "127.0.0.1"
#define SERVER_IP "127.0.0.1"

void send_packet(int sock, struct sockaddr_in &dest_addr, uint32_t seq, uint32_t ack_seq, bool syn, bool ack) {
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    struct iphdr *ip = (struct iphdr *) packet;
    struct tcphdr *tcp = (struct tcphdr *) (packet + sizeof(struct iphdr));

    // Fill IP header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(packet));
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr(CLIENT_IP);
    ip->daddr = inet_addr(SERVER_IP);

    // Fill TCP header
    tcp->source = htons(CLIENT_PORT); 
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(seq);
    tcp->ack_seq = htonl(ack_seq);
    tcp->doff = 5;  
    tcp->syn = syn ? 1 : 0;
    tcp->ack = ack ? 1 : 0;
    tcp->fin = 0;
    tcp->rst = 0;
    tcp->psh = 0;
    tcp->window = htons(8192);
    tcp->check = 0; // Kernel will compute the checksum

    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto() failed");
        exit(EXIT_FAILURE);
    } else {
        if(syn && !ack)
            std::cout << "[+] Sent SYN packet" << std::endl;
        else if(syn && ack) // should never happen
            std::cout << "[+] Sent SYN-ACK packet" << std::endl;
        else if(ack)
            std::cout << "[+] Sent final ACK packet" << std::endl;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // enable IP header inclusion
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    // set up the destination server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // as per server code, we should use sequence number 200 for the SYN
    // SYN flag is set to 1, ACK flag is set to 0
    send_packet(sock, server_addr, 200, 0, true, false);

    // create a timeout for receiving the SYN-ACK response
    struct timeval timeout;
    timeout.tv_sec = 5; // 5 seconds
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
            if (errno == EWOULDBLOCK || errno == EAGAIN) {  // timeout
                std::cerr << "[-] Response from server timed out." << std::endl;
                exit(EXIT_FAILURE);
            } else { // some other error
                perror("recv failed");
                continue;
            }
        }

        struct iphdr *ip = (struct iphdr *) buffer;
        struct tcphdr *tcp = (struct tcphdr *) (buffer + ip->ihl * 4);

        // ignore packets not destined for our client port
        if (ntohs(tcp->dest) != CLIENT_PORT) {  
            continue;
        }

        // check if it's the SYN-ACK response: SYN and ACK flags set
        // seq number should be 400 and ack number should be 201
        if (tcp->syn == 1 && tcp->ack == 1) {
            uint32_t server_seq = ntohl(tcp->seq);
            uint32_t server_ack = ntohl(tcp->ack_seq);
            std::cout << "[+] Received packet: SYN=" << (int)tcp->syn
                      << " ACK=" << (int)tcp->ack
                      << " SEQ=" << server_seq
                      << " ACK_SEQ=" << server_ack << std::endl;

            if (server_seq == 400 && server_ack == 201) {
                std::cout << "[+] SYN-ACK from server recognized." << std::endl;
                break;
            }
        }
    }

    // the client must now send an ACK packet with:
    // seq number 600 and ack number 401
    send_packet(sock, server_addr, 600, 401, false, true);

    std::cout << "[+] Handshake complete, closing socket." << std::endl;
    close(sock);
    return 0;
}
