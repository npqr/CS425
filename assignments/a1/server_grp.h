#ifndef SERVER_GRP_H
#define SERVER_GRP_H

#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define BUFFER_SIZE 1024
#define PORT 12345

typedef const std::string &cstr;
typedef const int &ci;

extern std::mutex clients_mutex;
extern std::unordered_map<int, std::string> clients;                            // Maps socket to username
extern std::unordered_map<std::string, std::unordered_set<int>> groups;         // Maps group name to set of client sockets
extern std::unordered_map<std::string, std::string> user_credentials;           // Stores username-password pairs
extern std::unordered_map<std::string, std::unordered_set<std::string>> users;  // Maps username to set of groups

std::string trim(cstr str);
void load_credentials();
void send_message(cstr message, ci client_socket);
void broadcast_message(cstr message, ci sender_socket);
void private_message(cstr recipient, cstr message, ci sender_socket);
void group_message(cstr group_name, cstr message, ci sender_socket);
void list_commands(ci client_socket);
void list_groups(cstr username, ci client_socket);
void list_members(cstr group_name, ci client_socket);
void create_group(cstr group_name, cstr username, ci client_socket);
void join_group(cstr group_name, cstr username, ci client_socket);
void leave_group(cstr group_name, cstr username, ci client_socket);
void handle_client(ci client_socket);
void sigint_handler(int signum);

#endif // SERVER_GRP_H