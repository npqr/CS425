#include "server_grp.h"

#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>

#include <algorithm>
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

std::mutex clients_mutex;
std::unordered_map<int, std::string> clients;                            // Maps socket to username
std::unordered_map<std::string, std::unordered_set<int>> groups;         // Maps group name to set of client sockets
std::unordered_map<std::string, std::string> user_credentials;           // Stores username-password pairs
std::unordered_map<std::string, std::unordered_set<std::string>> users;  // Maps username to set of groups

// Trim leading and trailing whitespaces
std::string trim(cstr str) {
    std::string _str = str;
    _str.erase(0, _str.find_first_not_of(" \n\r\t"));
    _str.erase(_str.find_last_not_of(" \n\r\t") + 1);
    return _str;
}

// Load user credentials from "users.txt"
void load_credentials() {
    std::ifstream file("users.txt");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password;
        if (std::getline(iss, username, ':') && std::getline(iss, password)) {
            password = trim(password);
            user_credentials[username] = password;
        }
    }
}

// Utility function to send a message to a client
void send_message(cstr message, ci client_socket) {
    send(client_socket, message.c_str(), message.size(), 0);
}

// Create a group
void create_group(cstr group_name, cstr username, ci client_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (groups.find(group_name) == groups.end()) {
        groups[group_name] = {client_socket};
        users[username].insert(group_name);
        send_message("Group " + group_name + " created.\n", client_socket);
    } else {
        send_message("Error: Group " + group_name + " already exists!\n", client_socket);
    }
}

// Join a group
void join_group(cstr group_name, cstr username, ci client_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (groups.find(group_name) != groups.end()) {
        groups[group_name].insert(client_socket);
        users[username].insert(group_name);
        send_message("Joined group " + group_name + ".\n", client_socket);
        for (auto &member : groups[group_name]) {
            if (member != client_socket)
                send_message("User " + username + " joined group " + group_name + ".", member);
        }
    } else {
        send_message("Error: Group " + group_name + " does not exist!\n", client_socket);
    }
}

// Leave a group
void leave_group(cstr group_name, cstr username, ci client_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (groups.find(group_name) != groups.end()) {
        if (groups[group_name].find(client_socket) == groups[group_name].end()) {
            send_message("You already are not a member of this group.\n", client_socket);
            return;
        }
        groups[group_name].erase(client_socket);
        users[username].erase(group_name);
        send_message("Left group " + group_name + ".\n", client_socket);
        for (auto &member : groups[group_name]) {
            if (member != client_socket)
                send_message("User " + username + " left the group" + group_name + ".", member);
        }
    } else {
        send_message("Error: Group " + group_name + " does not exist!\n", client_socket);
    }
}

// Broadcast message to all connected clients
void broadcast_message(cstr message, ci sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &[socket, username] : clients) {
        if (socket != sender_socket) {
            send_message(message, socket);
        }
    }
}

// Send a private message to a specific user
void private_message(cstr recipient, cstr message, ci sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &[socket, username] : clients) {
        if (username == recipient) {
            send_message(message, socket);
            return;
        }
    }
    send_message("Error: User " + recipient + " not found!\n", sender_socket);
}

// Send a message to all members of a group
void group_message(cstr group_name, cstr message, ci sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (groups.find(group_name) == groups.end()) {
        send_message("Error: Group does not exist!\n", sender_socket);
        return;
    }
    if (groups[group_name].find(sender_socket) == groups[group_name].end()) {
        send_message("Error: You are not a member of this group!\n", sender_socket);
        return;
    }
    for (int socket : groups[group_name]) {
        if (socket != sender_socket) {
            send_message("[" + group_name + "] @" + clients[sender_socket] + " : " + message, socket);
        }
    }
}

// List all commands
void list_commands(ci client_socket) {
    std::string commands = "/broadcast <message> : Broadcast message to all connected clients\n";
    commands += "/msg <username> <message> : Send a private message to a specific user\n";
    commands += "/group_msg <group_name> <message> : Send a message to all members of a group\n";
    commands += "/create_group <group_name> : Create a new group\n";
    commands += "/join_group <group_name> : Join an existing group\n";
    commands += "/leave_group <group_name> : Leave a group\n";
    commands += "/list_members <group_name> : List all members of a group\n";
    commands += "/list_groups : List all groups in which the user is a member\n";
    commands += "/list_commands : List all commands (to print this)\n";
    commands += "/exit : Exit the server\n";
    send_message(commands, client_socket);
}

// List all groups of a user
void list_groups(cstr username, ci client_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::string group_list = "You are in the following groups:\n";
    for (auto &group : users[username]) {
        group_list += group + "\n";
    }
    if (users[username].size() == 0) {
        group_list = "You are not a member of any group.\n";
    }
    send_message(group_list, client_socket);
}

// List all members of a group
void list_members(cstr group_name, ci client_socket) {
    if (groups.find(group_name) == groups.end()) {
        send_message("Error: Group " + group_name + " does not exist!\n", client_socket);
        return;
    }
    if (groups[group_name].find(client_socket) == groups[group_name].end()) {
        send_message("Error: You are not a member of this group!\n", client_socket);
        return;
    }
    std::string members = "Members of group " + group_name + ":\n";
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        if (groups.find(group_name) != groups.end()) {
            for (int socket : groups[group_name]) {
                members += clients[socket] + "\n";
            }
            send_message(members, client_socket);
        }
    }
}

// Command handlers for group commands
std::unordered_map<std::string, std::function<void(cstr, cstr, ci)>> grpcmd = {
    {"/group_msg", group_message},
    {"/create_group", create_group},
    {"/join_group", join_group},
    {"/leave_group", leave_group},
};

// Handle individual client connection
void handle_client(ci client_socket) {
    char buffer[BUFFER_SIZE];

    // Authentication
    send_message("Enter username: ", client_socket);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    // std::cout << "BBBBBBB" << buffer << "CCCCCCC\n";
    std::string username = trim(buffer);
    // std::cout << "@@@@@@" << username << "$$$$$$$\n";


    send_message("Enter password: ", client_socket);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    // std::cout << "DDDDDDD" << buffer << "EEEEEEE\n";
    std::string password = trim(buffer);
    // std::cout << "#######" << password << "^^^^^^^^\n";

    if ((user_credentials.find(username) == user_credentials.end()) || (user_credentials[username] != password)) {
        send_message("Authentication failed.\n", client_socket);
        close(client_socket);
        return;
    }

    // Do not allow multiple connections
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto &[socket, user] : clients) {
            if (user == username) {
                send_message("Error: User already connected!\n", client_socket);
                close(client_socket);
                return;
            }
        }
    }

    // Add client to the list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_socket] = username;
        for (const auto &group_name : users[username]) {
            groups[group_name].insert(client_socket);
        }
    }

    send_message("Welcome to the server, " + username + "!\n", client_socket);
    broadcast_message("User " + username + " joined the server!", client_socket);

    // Handle client commands
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(client_socket);
            for (auto &[group_name, members] : groups) {
                members.erase(client_socket);
            }
            close(client_socket);
            break;
        }

        std::string command = trim(buffer);
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        if (cmd == "/broadcast") {
            std::string message;
            std::getline(iss, message);
            broadcast_message("(broadcast) @" + username + " : " + trim(message), client_socket);
        } else if (cmd == "/msg") {
            std::string recipient, message;
            iss >> recipient;
            std::getline(iss, message);
            private_message(recipient, "(private) @" + username + " : " + trim(message), client_socket);
        } else if (grpcmd.find(cmd) != grpcmd.end() || cmd == "/list_members") {
            std::string group_name;
            iss >> group_name;
            std::string message;
            std::getline(iss, message);
            message = trim(message);

            if (cmd == "/list_members") {
                list_members(group_name, client_socket);
            } else if(cmd == "/group_msg") {
                grpcmd[cmd](group_name, message, client_socket);
            } else {
                grpcmd[cmd](group_name, username, client_socket);
            }
        } else if (cmd == "/list_groups") {
            list_groups(username, client_socket);
        } else if (cmd == "/list_commands") {
            list_commands(client_socket);
        } else if (cmd == "/exit") {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(client_socket);
                for (auto &[group_name, members] : groups) {
                    members.erase(client_socket);
                }
            }
            broadcast_message("User " + username + " left the server. :/", client_socket);
            close(client_socket);
            break;
        } else {
            send_message("Error: Unknown command ( " + cmd.substr(0, 10) + ((cmd.size() > 10) ? "... " : " ") + "). Run /list_commands to know the list of commands!\n", client_socket);
        }
    }
}

std::atomic<bool> running(true);

void sigint_handler(int signum) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::cout << "Server shutting down..." << std::endl;
    running = false;
}

#ifndef UNIT_TEST
int main() {
    signal(SIGINT, sigint_handler);

    int server_socket;
    sockaddr_in server_address{};

    load_credentials();

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error: Cannot create socket." << std::endl;
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error: Cannot bind socket." << std::endl;
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        std::cerr << "Error: Cannot listen on socket." << std::endl;
        close(server_socket);
        return 1;
    }

    fd_set read_fds;
    std::cout << "Server listening on port " << PORT << std::endl;

    while (running) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);

        struct timeval timeout = {1, 0};
        int activity = select(server_socket + 1, &read_fds, nullptr, nullptr, &timeout);

        if (activity == -1) {
            if (running)
                std::cerr << "Error: select error." << std::endl;
            break;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            sockaddr_in client_address{};
            socklen_t client_len = sizeof(client_address);
            int client_socket = accept(server_socket, (sockaddr *)&client_address, &client_len);
            if (client_socket < 0) {
                std::cerr << "Error: Cannot accept client connection." << std::endl;
                continue;
            }

            std::thread(handle_client, client_socket).detach();
        }
    }

    broadcast_message("Server shutting down... Please /exit to close your client.", server_socket);
    close(server_socket);
    return 0;
}
#endif