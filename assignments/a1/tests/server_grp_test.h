#ifndef SERVER_GRP_TEST_H
#define SERVER_GRP_TEST_H

#include <arpa/inet.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using namespace testing;

extern std::string username[2];
extern std::string group_name;

extern int server_socket, client_socket[2];
extern struct sockaddr_in server_addr, client_addr[2];
extern socklen_t addr_len;
extern int port;

extern std::unordered_map<std::string, std::unordered_set<int>> groups;
extern std::unordered_map<std::string, std::unordered_set<std::string>> users;
extern std::unordered_map<int, std::string> clients;

void init();
int generate_random_port();
void setup_server_socket(int& server_socket, int port, struct sockaddr_in& server_addr);
void setup_client_socket(int& client_socket, int port, struct sockaddr_in& server_addr, const std::string& username);
inline void accept_and_handle(int server_socket, struct sockaddr_in* client_addr, std::function<void(int)> handler);
void add_user_to_group(const std::string& group_name, int client_socket, const std::string& username);
void cleanup();

#endif // SERVER_GRP_TEST_H