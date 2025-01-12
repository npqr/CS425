#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>

#define BUFFER_SIZE 4096

namespace fs = std::filesystem;

std::atomic<bool> running(true);

std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

std::string getMimeType(const std::string& path) {
    if (!(fs::exists(path) && fs::is_regular_file(path))) {
        return "text/html";  // 404 or index page
    }

    std::map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".json", "application/json"},
        {".txt", "text/plain"}};

    std::string::size_type dotPos = path.rfind('.');
    if (dotPos != std::string::npos) {
        std::string ext = path.substr(dotPos);
        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end()) {
            return it->second;
        }
    }
    return "application/octet-stream";
}

std::string serveFile(std::string& filePath, int& statusCode, const std::string& staticDir) {
    if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
        std::ifstream file(filePath, std::ios::binary);
        std::ostringstream content;
        content << file.rdbuf();
        statusCode = 200;
        return content.str();
    } else if (fs::exists(filePath) && fs::is_directory(filePath)) {
        std::string indexPath = filePath + "index.html";  // we can later add for .php etc

        if (fs::exists(indexPath) && fs::is_regular_file(indexPath)) {
            return serveFile(indexPath, statusCode, staticDir);
        }

        std::ostringstream content;

        content << "<html><body><h1>Directory Listing</h1><ul>";

        if (fs::exists(filePath) && fs::is_directory(filePath)) {
            statusCode = 200;

            for (const auto& entry : fs::directory_iterator(filePath)) {
                const auto& path = entry.path();
                std::cout << path.string() << "\n";
                std::cout << filePath << "\n";
                if (fs::is_regular_file(path)) {
                    content << "<li><a href=\"" << path.filename().string() << "\">"
                            << path.filename().string() << "</a></li>";
                }
            }
        } else {
            statusCode = 404;
            content << "<li>Directory not found or invalid path.</li>";  // code won't reach here but anyway (if deleted in-transit)
        }

        content << "</ul></body></html>";

        return content.str();
    } else {
        std::string filePath404 = "static/404.html";  // assuming we have a 404 in /static
        // note that this is different from the staticDir

        if (fs::exists(filePath404) && fs::is_regular_file(filePath404)) {
            std::ifstream file404(filePath404, std::ios::binary);
            if (!file404.is_open()) {
                statusCode = 500;
                return "<html><body><h1>500 Internal Server Error</h1></body></html>";
            }

            std::ostringstream content;
            content << file404.rdbuf();
            statusCode = 404;
            return content.str();
        } else {
            statusCode = 404;

            // code will reach here if filePath404 is changed to staticDir + "404.html" and 404 is not in our static folder
            return "<html><body><h1>Too Ironic! Even 404 Not Found</h1></body></html>";
        }
    }
}

void logRequest(const std::string& clientIp, int clientPort, const std::string& request, int statusCode) {
    std::string timestamp = getCurrentTimestamp();
    std::string logEntry = "[" + timestamp + "] [" + clientIp + ":" + std::to_string(clientPort) + "] \"" +
                           request + "\" " + std::to_string(statusCode) + "\n";

    std::cout << logEntry;

    std::ofstream logFile("server.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << logEntry;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file for writing.\n";
    }
}

void sigintHandler(int sig) {
    std::cout << "\nReceived SIGINT (Ctrl+C), Shutting down the server!" << std::endl;
    running = false;
}

void handleClient(int clientSocket, const std::string& staticDir, sockaddr_in& clientAddr, int& retFlag) {
    retFlag = 1;
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
    if (bytesRead < 0) {
        perror("Read failed");
        close(clientSocket);
        {
            retFlag = 3;
            return;
        };
    }

    std::string request(buffer);
    std::cout << "Request received:\n"
              << request;

    std::istringstream requestStream(request);
    std::string method, path, version;
    requestStream >> method >> path >> version;

    if (path == "/") path = "/index.html";
    std::string filePath = staticDir + path;

    int statusCode;
    std::string responseBody = serveFile(filePath, statusCode, staticDir);

    // while routing a directory, if trailing slash is skipped, browser thinks of it as a file
    // crude fix, but works at the moment
    // we redirect it with a trailing slash

    if (fs::exists(filePath) && fs::is_directory(filePath) && filePath.back() != '/') {
        std::string newLocation = path + "/";
        std::string redirectResponse =
            "HTTP/1.1 301 Moved Permanently\r\n"
            "Location: " +
            newLocation +
            "\r\n"
            "\r\n";
        send(clientSocket, redirectResponse.c_str(), redirectResponse.size(), 0);
        close(clientSocket);
        return;
    }

    // Anatomy of an HTTP Message
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages
    std::string response =
        "HTTP/1.1 " + std::to_string(statusCode) + " " + (statusCode == 200 ? "OK" : "Not Found") + "\r\n" +
        "Content-Type: " + getMimeType(filePath) + "\r\n" +
        "Content-Length: " + std::to_string(responseBody.size()) + "\r\n" +
        "\r\n" +
        responseBody;

    send(clientSocket, response.c_str(), response.size(), 0);
    logRequest(inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), method + " " + path + " " + version, statusCode);
    close(clientSocket);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, sigintHandler);  // for Ctrl + C - shut down

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <static_directory>\n";
        return EXIT_FAILURE;
    }

    int port = std::stoi(argv[1]);
    std::string staticDir = argv[2];

    if (!fs::exists(staticDir) || !fs::is_directory(staticDir)) {
        std::cerr << "Error: Static directory does not exist or is invalid.\n";
        return EXIT_FAILURE;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // AF_INET == ipv4
    if (serverSocket == -1) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // htons, htonl, ntohs, ntohl reference
    // from https://beej.us/guide/bgnet/html/split/ip-addresses-structs-and-data-munging.html

    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    // Reference for select(), fd_set, FD_ZERO stuff
    // https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html#select

    fd_set readfds;

    std::cout << "Server started on port " << port << "\n";

    while (running) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        struct timeval timeout = {1, 0};
        int activity = select(serverSocket + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity == -1) {
            if (running)
                perror("Select Error");
            break;
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            sockaddr_in clientAddr{};
            socklen_t clientLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSocket < 0) {
                perror("Accept failed");
                continue;
            }

            int retFlag;

            std::thread clientThread([clientSocket, &staticDir, &clientAddr, &retFlag]() {
                handleClient(clientSocket, staticDir, clientAddr, retFlag);
            });

            clientThread.detach();
            if (retFlag == 3) continue;
        }
    }

    close(serverSocket);
    return 0;
}

/*  
    Author: npqr
    22:22 12-01-2025
*/