#ifndef SERVER_GRP_H
#include "../server_grp.h"
#endif

#include <arpa/inet.h>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
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

#include "server_grp_test.h"

using namespace testing;

std::string username[2] = {"user1", "user2"};
std::string group_name = "test_group";

int server_socket, client_socket[2];
struct sockaddr_in server_addr, client_addr[2];
socklen_t addr_len = sizeof(struct sockaddr_in);
int port;

void init() {
    port = generate_random_port();
    setup_server_socket(server_socket, port, server_addr);
    setup_client_socket(client_socket[0], port, server_addr, username[0]);
    setup_client_socket(client_socket[1], port, server_addr, username[1]);
}

int generate_random_port() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(BUFFER_SIZE, 65535);
    return dis(gen);
}

void setup_server_socket(int& server_socket, int port, struct sockaddr_in& server_addr) {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(server_socket, -1) << "Failed to create server socket";

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    ASSERT_EQ(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)), 0) << "Failed to bind server socket";
    ASSERT_EQ(listen(server_socket, 10), 0) << "Failed to listen on server socket";
}

void setup_client_socket(int& client_socket, int port, struct sockaddr_in& server_addr, const std::string& username) {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(client_socket, -1) << "Failed to create client socket";

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    ASSERT_EQ(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)), 0) << "Failed to connect client socket";
    clients[client_socket] = username;
}

inline void accept_and_handle(int server_socket, struct sockaddr_in* client_addr, std::function<void(int)> handler) {
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int connection_socket = accept(server_socket, (struct sockaddr*)client_addr, &addr_len);
    ASSERT_NE(connection_socket, -1) << "Failed to accept connection";
    handler(connection_socket);
    close(connection_socket);
}

void add_user_to_group(const std::string& group_name, int client_socket, const std::string& username) {
    groups[group_name].insert(client_socket);
    users[username].insert(group_name);
}

void cleanup() {
    close(server_socket);
    close(client_socket[0]);
    close(client_socket[1]);
    clients.clear();
    groups.clear();
    users.clear();
}

TEST(ServerGrpTest, LoadCredentials) {
    std::ofstream file("users.txt");
    file << "user1:password1\n";
    file << "user2:password2\n";
    file.close();

    load_credentials();

    EXPECT_EQ(user_credentials["user1"], "password1");
    EXPECT_EQ(user_credentials["user2"], "password2");

    std::remove("users.txt");
}

TEST(ServerGrpTest, SendAndReceiveMessage) {
    init();

    std::thread client_thread([&]() {
        send_message("Hello, World!", client_socket[0]);
    });

    if (client_thread.joinable()) client_thread.join();

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        char buffer[50] = {0};
        recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
        EXPECT_STREQ(buffer, "Hello, World!");
    });

    cleanup();
}

TEST(ServerGrpTest, CreateGroup) {
    init();

    std::thread client_thread([&]() {
        create_group(group_name, username[0], client_socket[0]);
    });

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        char buffer[BUFFER_SIZE] = {0};
        recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
        EXPECT_STREQ(buffer, ("Group " + group_name + " created.\n").c_str());
    });

    if (client_thread.joinable()) client_thread.join();

    EXPECT_TRUE(groups.find(group_name) != groups.end());
    EXPECT_TRUE(groups[group_name].find(client_socket[0]) != groups[group_name].end());
    EXPECT_TRUE(users[username[0]].find(group_name) != users[username[0]].end());

    cleanup();
}

TEST(ServerGrpTest, JoinGroup) {
    init();

    add_user_to_group(group_name, client_socket[0], username[0]);

    std::thread client_thread([&]() {
        join_group(group_name, username[1], client_socket[1]);

        accept_and_handle(server_socket, &client_addr[1], [&](int connection_socket) {
            char buffer[BUFFER_SIZE] = {0};
            recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
            EXPECT_STREQ(buffer, ("Joined group " + group_name + ".\n").c_str());
        });
    });

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        char buffer[BUFFER_SIZE] = {0};
        recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
        EXPECT_STREQ(buffer, ("User " + username[1] + " joined group " + group_name + ".").c_str());
    });

    if (client_thread.joinable()) client_thread.join();

    EXPECT_TRUE(groups[group_name].find(client_socket[1]) != groups[group_name].end());
    EXPECT_TRUE(users[username[1]].find(group_name) != users[username[1]].end());

    cleanup();
}

TEST(ServerGrpTest, LeaveGroup) {
    init();

    add_user_to_group(group_name, client_socket[0], username[0]);
    add_user_to_group(group_name, client_socket[1], username[1]);

    std::thread client_thread([&]() {
        leave_group(group_name, username[1], client_socket[1]);
    });

    accept_and_handle(server_socket, &client_addr[1], [&](int connection_socketx) {
        accept_and_handle(server_socket, &client_addr[1], [&](int connection_socket) {
            char buffer[BUFFER_SIZE] = {0};
            recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
            EXPECT_STREQ(buffer, "Left group test_group.\n");
        });
    });

    if (client_thread.joinable()) client_thread.join();

    EXPECT_TRUE(groups[group_name].find(client_socket[1]) == groups[group_name].end());
    EXPECT_TRUE(users[username[1]].find(group_name) == users[username[1]].end());

    cleanup();
}

TEST(ServerGrpTest, BroadcastMessage) {
    init();

    std::thread client_thread1([&]() {
        broadcast_message("Broadcast message", client_socket[0]);
    });

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        accept_and_handle(server_socket, &client_addr[1], [&](int connection_socket) {
            char buffer[BUFFER_SIZE] = {0};
            recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
            EXPECT_STREQ(buffer, "Broadcast message");
        });
    });

    if (client_thread1.joinable()) client_thread1.join();
    cleanup();
}

TEST(ServerGrpTest, PrivateMessage) {
    init();

    std::thread client_thread([&]() {
        private_message("user2", "(private) @user1 : hi in private!", client_socket[0]);
    });

    if (client_thread.joinable()) client_thread.join();

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        accept_and_handle(server_socket, &client_addr[1], [&](int connection_socket) {
            char buffer[BUFFER_SIZE] = {0};
            recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
            EXPECT_STREQ(buffer, "(private) @user1 : hi in private!");
        });
    });

    cleanup();
}

TEST(ServerGrpTest, GroupMessage) {
    init();

    add_user_to_group(group_name, client_socket[0], username[0]);
    add_user_to_group(group_name, client_socket[1], username[1]);

    std::thread client_thread([&]() {
        group_message(group_name, "Group message", client_socket[0]);
    });

    if (client_thread.joinable()) client_thread.join();

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        accept_and_handle(server_socket, &client_addr[1], [&](int connection_socket) {
            char buffer[BUFFER_SIZE] = {0};
            recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
            EXPECT_STREQ(buffer, "[test_group] @user1 : Group message");
        });
    });

    cleanup();
}

TEST(ServerGrpTest, ListCommands) {
    init();

    std::thread client_thread([&]() {
        list_commands(client_socket[0]);
    });

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        char buffer[BUFFER_SIZE] = {0};
        recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
        EXPECT_TRUE(std::string(buffer).find("/broadcast") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/msg") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/group_msg") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/create_group") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/join_group") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/leave_group") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/list_members") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/list_groups") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/list_commands") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("/exit") != std::string::npos);
    });

    if (client_thread.joinable()) client_thread.join();

    cleanup();
}

TEST(ServerGrpTest, ListGroups) {
    init();

    add_user_to_group("group1", client_socket[0], username[0]);
    add_user_to_group("group2", client_socket[0], username[0]);

    std::thread client_thread([&]() {
        list_groups(username[0], client_socket[0]);
    });

    if (client_thread.joinable()) client_thread.join();

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        char buffer[BUFFER_SIZE] = {0};
        recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
        EXPECT_TRUE(std::string(buffer).find("group1") != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find("group2") != std::string::npos);
    });

    cleanup();
}

TEST(ServerGrpTest, ListMembers) {
    init();

    add_user_to_group(group_name, client_socket[0], username[0]);
    add_user_to_group(group_name, client_socket[1], username[1]);

    std::thread client_thread([&]() {
        list_members(group_name, client_socket[0]);
    });

    if (client_thread.joinable()) client_thread.join();

    accept_and_handle(server_socket, &client_addr[0], [&](int connection_socket) {
        char buffer[BUFFER_SIZE] = {0};
        recv(connection_socket, buffer, sizeof(buffer) - 1, 0);
        EXPECT_TRUE(std::string(buffer).find(username[0]) != std::string::npos);
        EXPECT_TRUE(std::string(buffer).find(username[1]) != std::string::npos);
    });

    cleanup();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
