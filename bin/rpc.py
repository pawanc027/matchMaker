import socket
import re

matching_server = ("localhost", 8800)
storage_server = ("localhost", 8801)

def do_api(server, request, verbose=True):
    request_str = format_message(request)

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server)

    if verbose:
        print("{}### {}:{}\n{}".format("\x1b[01;36m", "sending", "\x1b[0m", request_str.rstrip()))

    s.sendall(request_str.encode("utf8"))
    s.shutdown(socket.SHUT_WR)

    response_str = recvall(s).decode("utf8")

    s.close()

    if verbose:
        print("{}### {}:{}\n{}\n".format("\x1b[01;31m", "received", "\x1b[0m", response_str.rstrip()))

    return parse_message(response_str)

def recvall(s):
    """calls recv repeatedly until EOF"""
    response_bytes = "".encode("utf8")
    while True:
        chunk = s.recv(0x1000)
        if len(chunk) == 0:
            break
        response_bytes += chunk
    return response_bytes

def format_message(m):
    error_suffix = ". example message stucture: [{'k':'v'}]. found this message: " + repr(m)
    # sanity type checking
    if not (type(m) == list):
        raise TypeError("expected message to be a list" + error_suffix)
    if not all(type(d) == dict for d in m):
        raise TypeError("expected objects in message to be dict" + error_suffix)
    def is_str(o):
        # compatible with python2(str, unicode) and python3(str) and not python3(bytes)
        return hasattr(o, "encode")
    if not all(is_str(k) for d in m for k in d.keys()):
        raise TypeError("all keys must be type str" + error_suffix)
    if not all(is_str(v) for d in m for v in d.values()):
        raise TypeError("all values must be type str" + error_suffix)

    assert all(re.match('^[^\n:]*$', k) for d in m for k in d.keys())
    assert all(re.match('^[^\n]*$', v) for d in m for v in d.values())
    return "\n\n".join("\n".join("{}:{}".format(k, v) for (k, v) in d.items()) for d in m)

def parse_message(s):
    # example s:
    "k1:v1\nk2:v2\n\nk1:v1\nk2:v2\n\n"

    m = []
    for blob in s.split("\n\n"):
        d = {}
        for line in blob.split("\n"):
            try:
                k, v = line.split(":", 1)
            except ValueError:
                # no ':' in this line
                return None
            d[k] = v
        m.append(d)

    return m
