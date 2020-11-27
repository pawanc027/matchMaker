#include "rpc.hpp"
#include "matching-handler.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>

static bool start_server(int *server_fd) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("ERROR: failed to create socket");
        return false;
    }
    // we never close the socket.

    int value = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0) {
        perror("ERROR: failed to setsockopt");
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MATCHING_SERVICE_PORT);
    if (bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        perror("ERROR: failed to bind");
        return false;
    }

    if (listen(fd, 5)) {
        perror("ERROR: failed to listen");
        return false;
    }
    std::cout << "listening on port: " << MATCHING_SERVICE_PORT << "\n";

    *server_fd = fd;

    return true;
}

int main() {
    int server_fd;
    if (!start_server(&server_fd)) {
        return 1;
    }

    while (true) {
        std::cout << "\n";
        int client_fd = accept(server_fd, nullptr, 0);
        if (client_fd < 0) {
            perror("ERROR: failed to accept");
            continue;
        }
        std::cout << "\x1b[01;31m" << "### handling request" << "\x1b[0m" << "\n";

        message request;
        if (!read_message(client_fd, request)) {
            std::cout << "malformed request message\n";
            close(client_fd);
            continue;
        }

        message response;
        if (!handle_request(request, response)) {
            std::cout << "request not handled properly\n";
            close(client_fd);
            continue;
        }

        std::cout << "\x1b[01;36m" << "### writing response" << "\x1b[0m" << "\n";
        if (!write_message(client_fd, response)) {
            std::cout << "error while writing response\n";
            close(client_fd);
            continue;
        }

        close(client_fd);
    }
    // this program runs forever.
    // kill it with Ctrl+C/SIGINT.
}
