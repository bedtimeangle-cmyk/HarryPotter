/**
 * @file HttpServer.h
 * @brief 简易 HTTP 服务器（仅欣求GET/POST）
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <functional>
#include <map>

using HttpHandler = std::function<std::string(const std::map<std::string, std::string>&)>;

class HttpServer {
public:
    HttpServer(int port = 8080);
    ~HttpServer();

    void registerGetHandler(const std::string& path, HttpHandler handler);
    void registerPostHandler(const std::string& path, HttpHandler handler);

    void start();
    void stop();

private:
    int port_;
    bool running_ = false;
    std::map<std::string, HttpHandler> getHandlers_;
    std::map<std::string, HttpHandler> postHandlers_;

    void handleConnection(int clientSocket);
    std::map<std::string, std::string> parseRequest(const std::string& request);
};

#endif  // HTTP_SERVER_H
