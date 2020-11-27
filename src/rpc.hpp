#pragma once

#include <stdint.h>
#include <map>
#include <vector>
#include <string>

static const uint16_t MATCHING_SERVICE_PORT = 8800;
static const uint16_t STORAGE_SERVER_PORT = 8801;

// See README.md for a description of the message data type.
typedef std::map<std::string, std::string> string_map;
typedef std::vector<string_map> message;

// All of these APIs returns true on success.
// On success, output is written to the out_ parameter.
// On failure, the out_ parameter is left in an undefined state.
// See doc/API.md for behavioral documentation of these.
bool do_storage_get(const std::string & dir, const std::string & entry, string_map & out_object);
bool do_storage_get(const std::vector<std::pair<std::string, std::string>> & dirs_and_entries, message & out_message);
bool do_storage_list_dir(const std::string & dir, std::vector<std::string> & out_entries);
bool do_storage_put(const std::string & dir, const std::string & entry, const string_map & object);
bool do_storage_delete_everything();


// Returns true on success. Message is written to out_m.
bool read_message(int socket_fd, message & out_m);

// Returns true on success.
bool write_message(int socket_fd, const message & m);

// Returns true on success. Response is written to out_response.
bool do_api(uint16_t port, const message & request, message & out_response);

// Returns m[k], or if there is no key k present in the map, then returns default_.
std::string get_or_default(const string_map & m, const std::string & k, const std::string & default_);
// Like the above, but converts the value to an int. If the conversion fails, returns default_.
int get_or_default(const string_map & m, const std::string & k, int default_);

// Like python s.split(delimiter)
std::vector<std::string> split(const std::string & s, const std::string & delimiter);
// Like python s.split(delimiter, 1)
std::vector<std::string> split_once(const std::string & s, const std::string & delimiter);
// Like python s.endswith(suffix)
bool ends_with(const std::string & s, const std::string & suffix);
