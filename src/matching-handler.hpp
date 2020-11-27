#pragma once

#include "rpc.hpp"

// returns true on success
bool handle_request(const message & request, message & response);
