#include "rpc.hpp"

#include <sys/socket.h>
#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sstream>
#include <iostream>

using namespace std;

// see the header file for documentation for the functions here.

string get_or_default(const string_map & m, const string & k, const string & default_) {
    if (m.count(k) == 0) return default_;
    return m.at(k);
}
int get_or_default(const string_map & m, const string & k, int default_) {
    string value_str = get_or_default(m, k, "");
    if (value_str == "") return default_;
    size_t end_index;
    int i = stoi(value_str, &end_index);
    if (end_index != value_str.length()) return default_;
    return i;
}

static bool recv_all(int socket_fd, string & out_string) {
    stringstream accumulator;
    while (true) {
        char buf[0x1000];
        ssize_t count = recv(socket_fd, buf, sizeof(buf), 0);
        if (count < 0) {
            perror("unable to recv");
            return false;
        }

        if (count == 0) {
            break;
        }

        accumulator.write(buf, count);
    }

    out_string = accumulator.str();
    return true;
}

bool read_message(int socket_fd, message & out_m) {
    string s;
    if (!recv_all(socket_fd, s)) {
        return false;
    }

    vector<string> blobs = split(s, "\n\n");
    out_m.clear();
    for (string blob : blobs) {
        vector<string> lines = split(blob, "\n");

        out_m.push_back(string_map());
        for (const string& line : lines) {
            vector<string> key_value = split_once(line, ":");
            if (key_value.size() != 2) {
                cerr << "expected colon\n";
                return false;
            }

            out_m.back()[key_value[0]] = key_value[1];
        }
    }

    return true;
}

// calls send() repeatedly until all data is sent.
// returns true on success
static bool sendall(int socket_fd, const string& s) {
    const char * buf = s.c_str();
    size_t len = s.length();
    while (len > 0) {
        ssize_t signed_sent = send(socket_fd, buf, len, 0);
        if (signed_sent < 0) {
            perror("send failed");
            return false;
        }
        size_t sent = (size_t)signed_sent;
        assert(sent <= len);

        buf += sent;
        len -= sent;
    }
    return true;
}

// returns true on success
bool write_message(int socket_fd, const message & m) {
    bool need_blob_delimiter = false;
    for (string_map o : m) {
        if (need_blob_delimiter) {
            sendall(socket_fd, "\n\n");
        } else {
            need_blob_delimiter = true;
        }

        bool need_line_delimiter = false;
        for (auto it = o.begin(); it != o.end(); it++) {
            if (need_line_delimiter) {
                sendall(socket_fd, "\n");
            } else {
                need_line_delimiter = true;
            }
            string k = it->first;
            string v = it->second;
            if (!(sendall(socket_fd, k) &&
                  sendall(socket_fd, ":") &&
                  sendall(socket_fd, v))) {
                return false;
            }
        }
    }
    return true;
}

bool do_api(uint16_t port, const message & request, message & out_response) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("ERROR: failed to create socket");
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    if (connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        perror("ERROR: failed to connect");
        close(fd);
        return false;
    }

    if (!write_message(fd, request)) {
        close(fd);
        return false;
    }

    if (shutdown(fd, SHUT_WR) != 0) {
        perror("failed to shutdown wr");
        close(fd); // is this even going to work
        return false;
    }

    if (!read_message(fd, out_response)) {
        close(fd); // is this even going to work
        return false;
    }

    close(fd);
    return true;
}

bool do_storage_get(const string & dir, const string & entry, string_map & out_object) {
    message m;
    if (!do_storage_get({
        {dir, entry},
    }, m)) {
        return false;
    }
    assert(m.size() == 1); // already checked

    out_object.clear();
    if (get_or_default(m[0], "error", "") == "not_found") {
        // leave out_object empty
        return true;
    }
    out_object = m[0];
    return true;
}

bool do_storage_get(const vector<pair<string, string>> & dirs_and_entries, message & out_message) {
    message request;
    request.push_back({
        {"command", "get"},
    });
    for (auto pair : dirs_and_entries) {
        request.push_back({
            {"dir", pair.first},
            {"entry", pair.second},
        });
    }
    if (!do_api(STORAGE_SERVER_PORT, request, out_message)) {
        return false;
    }
    if (out_message.size() != dirs_and_entries.size()) {
        // even if some of them were errors, we should always get the same number
        cerr << "wrong number of objects returned from get\n";
        return false;
    }
    return true;
}

bool do_storage_list_dir(const string & dir, vector<string> & out_entries) {
    message m;
    if (!do_api(STORAGE_SERVER_PORT, {{
        {"command", "list_dir"},
        {"dir", dir},
    }}, m)) {
        return false;
    }
    out_entries.clear();
    if (m.size() == 1 && get_or_default(m[0], "error", "") == "none") {
        // push nothing to out_entries
        return true;
    }
    // push each entry
    for (string_map object : m) {
        out_entries.push_back(get_or_default(object, "entry", ""));
    }
    return true;
}

bool do_storage_put(const string & dir, const string & entry, const string_map & object) {
    message m;
    return do_api(STORAGE_SERVER_PORT, {
        {
            {"command", "put"},
            {"dir", dir},
            {"entry", entry},
        },
        object,
    }, m);
    // no meaningful data returned
}

bool do_storage_delete_everything() {
    message m;
    return do_api(STORAGE_SERVER_PORT, {{
        {"command", "delete_everything"},
    }}, m);
    // no meaningful data returned
}

// Like python s.split(delimiter)
vector<string> split(const string & s, const string & delimiter) {
    vector<string> result;
    size_t cursor = 0;
    while (true) {
        size_t index = s.find(delimiter, cursor);
        if (index == string::npos) {
            break;
        }
        result.push_back(s.substr(cursor, index - cursor));
        cursor = index + delimiter.length();
    }
    result.push_back(s.substr(cursor));
    return result;
}
// Like python s.split(delimiter, 1)
vector<string> split_once(const string & s, const string & delimiter) {
    size_t index = s.find(delimiter);
    if (index == string::npos) {
        // not found
        return { s };
    }
    return {
        s.substr(0, index),
        s.substr(index + delimiter.length()),
    };
}

bool ends_with(const string & s, const string & suffix) {
    if (s.length() < suffix.length()) return false;
    return s.find(suffix, s.length() - suffix.length()) != string::npos;
}
