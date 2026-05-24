/**
 * @file HttpServer.cpp
 * @brief 简易 HTTP 服务器实现
 */

#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
using socklen_t = int;
const socket_t INVALID_SOCKET_VAL = INVALID_SOCKET;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using socket_t = int;
const socket_t INVALID_SOCKET_VAL = -1;
const int SOCKET_ERROR = -1;
#endif

HttpServer::HttpServer(int port) : port_(port) {}

HttpServer::~HttpServer() { stop(); }

void HttpServer::registerGetHandler(const std::string& path,
                                     HttpHandler handler) {
    getHandlers_[path] = handler;
}

void HttpServer::registerPostHandler(const std::string& path,
                                      HttpHandler handler) {
    postHandlers_[path] = handler;
}

void HttpServer::start() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }
#endif

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET_VAL) {
        std::cerr << "socket() failed\n";
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
        SOCKET_ERROR) {
        std::cerr << "bind() failed\n";
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed\n";
        return;
    }

    running_ = true;
    std::cout << "HTTP Server listening on port " << port_ << "...\n";

    while (running_) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        socket_t clientSocket =
            accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET_VAL) continue;

        std::thread(&HttpServer::handleConnection, this, clientSocket).detach();
    }

#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#else
    close(serverSocket);
#endif
}

void HttpServer::stop() { running_ = false; }

void HttpServer::handleConnection(int clientSocket) {
    // 接收请求
    char buffer[4096] = {0};
    int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
        return;
    }

    std::string request(buffer);
    auto params = parseRequest(request);
    std::string method = params["method"];
    std::string path = params["path"];

    // 墨控仅支持 /api/* 路径
    std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404";

    if (method == "GET") {
        auto it = getHandlers_.find(path);
        if (it != getHandlers_.end()) {
            std::string body = it->second(params);
            response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Length: " + std::to_string(body.length()) +
                      "\r\n\r\n" + body;
        }
    } else if (method == "POST") {
        auto it = postHandlers_.find(path);
        if (it != postHandlers_.end()) {
            std::string body = it->second(params);
            response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Length: " + std::to_string(body.length()) +
                      "\r\n\r\n" + body;
        }
    }

    send(clientSocket, response.c_str(), response.length(), 0);

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
}

std::map<std::string, std::string> HttpServer::parseRequest(
    const std::string& request) {
    std::map<std::string, std::string> result;

    std::istringstream iss(request);
    std::string method, pathAndQuery, httpVersion;
    iss >> method >> pathAndQuery >> httpVersion;

    result["method"] = method;

    // 提取路径（去掉查询字符串）
    size_t queryPos = pathAndQuery.find('?');
    if (queryPos != std::string::npos) {
        result["path"] = pathAndQuery.substr(0, queryPos);
        std::string queryString = pathAndQuery.substr(queryPos + 1);

        // 解析查询参数
        std::istringstream queryIss(queryString);
        std::string param;
        while (std::getline(queryIss, param, '&')) {
            size_t eqPos = param.find('=');
            if (eqPos != std::string::npos) {
                std::string key = param.substr(0, eqPos);
                std::string value = param.substr(eqPos + 1);
                result[key] = value;
            }
        }
    } else {
        result["path"] = pathAndQuery;
    }

    // 批量提取 body
    size_t bodyPos = request.find("\r\n\r\n");
    if (bodyPos != std::string::npos) {
        result["body"] = request.substr(bodyPos + 4);
    }

    return result;
}
