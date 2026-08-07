#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "srl_net.hpp"
#include "vm.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#endif

#include <mutex>
#include <atomic>

namespace srl {

static std::once_flag g_winsockInitFlag;
static std::unordered_map<double, SOCKET> openSockets;
static double sockIdCounter = 1.0;
static std::mutex g_netMutex;

static void ensureWinsock() {
#ifdef _WIN32
    std::call_once(g_winsockInitFlag, []() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    });
#endif
}

void NET::registerNativeFunctions(VM& vm) {
    // net_http_get(url_host, path)
    vm.defineNative("net_http_get", [](int argCount, const Value* args) -> Value {
        ensureWinsock();
        if (argCount > 0 && args[0].isString()) {
            std::string host = args[0].asString();
            std::string path = (argCount > 1 && args[1].isString()) ? args[1].asString() : "/";

            // Remove http:// or https:// if present
            if (host.rfind("http://", 0) == 0) host = host.substr(7);
            else if (host.rfind("https://", 0) == 0) host = host.substr(8);

            size_t slashPos = host.find('/');
            if (slashPos != std::string::npos) {
                path = host.substr(slashPos);
                host = host.substr(0, slashPos);
            }

            struct addrinfo hints = {0}, *res = nullptr;
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            if (getaddrinfo(host.c_str(), "80", &hints, &res) != 0 || !res) {
                return Value("");
            }

            SOCKET s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if (s == INVALID_SOCKET) {
                freeaddrinfo(res);
                return Value("");
            }

            if (connect(s, res->ai_addr, (int)res->ai_addrlen) != 0) {
                closesocket(s);
                freeaddrinfo(res);
                return Value("");
            }
            freeaddrinfo(res);

            std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nUser-Agent: SRL-Lang/0.3.2\r\nConnection: close\r\n\r\n";
            send(s, req.c_str(), (int)req.length(), 0);

            std::string response;
            char buf[1024];
            int bytes;
            while ((bytes = recv(s, buf, sizeof(buf) - 1, 0)) > 0) {
                buf[bytes] = '\0';
                response += buf;
            }
            closesocket(s);
            return Value(response);
        }
        return Value("");
    });

    // net_tcp_connect(host, port)
    vm.defineNative("net_tcp_connect", [](int argCount, const Value* args) -> Value {
        ensureWinsock();
        if (argCount >= 2 && args[0].isString() && args[1].isNumber()) {
            std::string host = args[0].asString();
            std::string portStr = std::to_string(static_cast<int>(args[1].asNumber()));

            struct addrinfo hints = {0}, *res = nullptr;
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) == 0 && res) {
                SOCKET s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (s != INVALID_SOCKET) {
                    if (connect(s, res->ai_addr, (int)res->ai_addrlen) == 0) {
                        std::lock_guard<std::mutex> lock(g_netMutex);
                        double id = sockIdCounter++;
                        openSockets[id] = s;
                        freeaddrinfo(res);
                        return Value(id);
                    }
                    closesocket(s);
                }
                freeaddrinfo(res);
            }
        }
        return Value(0.0);
    });

    // net_tcp_send(sock_id, data)
    vm.defineNative("net_tcp_send", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isString()) {
            double id = args[0].asNumber();
            std::string data = args[1].asString();
            SOCKET s = INVALID_SOCKET;
            {
                std::lock_guard<std::mutex> lock(g_netMutex);
                auto it = openSockets.find(id);
                if (it != openSockets.end()) {
                    s = it->second;
                }
            }
            if (s != INVALID_SOCKET) {
                int sent = send(s, data.c_str(), static_cast<int>(data.length()), 0);
                return Value(static_cast<double>(sent));
            }
        }
        return Value(0.0);
    });

    // net_tcp_recv(sock_id, max_bytes)
    vm.defineNative("net_tcp_recv", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            double id = args[0].asNumber();
            int maxB = (argCount > 1 && args[1].isNumber()) ? static_cast<int>(args[1].asNumber()) : 1024;
            if (maxB <= 0) return Value("");
            SOCKET s = INVALID_SOCKET;
            {
                std::lock_guard<std::mutex> lock(g_netMutex);
                auto it = openSockets.find(id);
                if (it != openSockets.end()) {
                    s = it->second;
                }
            }
            if (s != INVALID_SOCKET) {
                std::vector<char> buf(maxB + 1);
                int bytes = recv(s, buf.data(), maxB, 0);
                if (bytes > 0) {
                    buf[bytes] = '\0';
                    return Value(std::string(buf.data(), bytes));
                }
            }
        }
        return Value("");
    });

    // net_tcp_close(sock_id)
    vm.defineNative("net_tcp_close", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            double id = args[0].asNumber();
            std::lock_guard<std::mutex> lock(g_netMutex);
            auto it = openSockets.find(id);
            if (it != openSockets.end()) {
                closesocket(it->second);
                openSockets.erase(it);
                return Value(true);
            }
        }
        return Value(false);
    });
}

} // namespace srl
