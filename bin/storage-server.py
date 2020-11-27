#!/usr/bin/env python

import rpc

import os
import re
import socket
import shutil

storage_root = os.path.join(os.path.dirname(os.path.dirname(__file__)), "data")

def main():

    # listen
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(rpc.storage_server)
    server_socket.listen(3)
    print("listening on: " + repr(rpc.storage_server))

    # accept connections forever
    while True:
        (client_socket, _) = server_socket.accept()
        try:
            request_str = rpc.recvall(client_socket).decode("utf8")
            print("{}### {}:{}\n{}".format("\x1b[01;31m", "received", "\x1b[0m", request_str.rstrip()))
            request = rpc.parse_message(request_str)
            if request == None:
                continue
            response = handle_request(request)
            if response == None:
                continue
            response_str = rpc.format_message(response)
            print("{}### {}:{}\n{}\n".format("\x1b[01;36m", "sending", "\x1b[0m", response_str.rstrip()))
            client_socket.sendall(response_str.encode("utf8"))
        finally:
            client_socket.close()

def handle_request(request):
    command = request[0].get("command", "")
    if command == "get":
        return handle_get(request)
    if command == "list_dir":
        return handle_list_dir(request)
    if command == "put":
        return handle_put(request)
    if command == "delete_everything":
        return handle_delete_everything(request)

    print("command not supported: " + command)
    return None

def handle_get(request):
    if len(request) < 2:
        return None

    response = []
    for request_o in request[1:]:
        path = get_storage_path(request_o)
        if path == None:
            # request is malformed. you get nothing.
            return None

        try:
            with open(path, "r") as f:
                # strip trailing newline
                contents = f.read()[:-1]
        except IOError:
            response.append({"error": "not_found"})
            continue

        try:
            (o,) = rpc.parse_message(contents)
        except ValueError:
            # this isn't really the client's fault.
            # give an error entry.
            response.append({"error": "malformed"})
            continue

        response.append(o)

    return response

def handle_list_dir(request):
    if len(request) != 1:
        return None
    storage_path = get_storage_path(request[0], include_entry=False)

    try:
        names = sorted(os.listdir(storage_path))
    except OSError:
        names = []

    response = []
    for name in names:
        if name.endswith(".txt"):
            response.append({"entry": name[:-len(".txt")]})

    if len(response) == 0:
        response = [{"error": "none"}]

    return response

def handle_put(request):
    if len(request) != 2:
        return None
    path = get_storage_path(request[0])
    if path == None:
        return None
    blob = {k:v for k,v in request[1].items() if k not in ("command", "dir", "entry")}
    if len(blob) == 0:
        print("empty blob")
        return None

    try:
        os.makedirs(os.path.dirname(path))
    except OSError:
        pass
    print("created directory: " + storage_root)
    with open(path, "w") as f:
        # add a trailing newline just to make using `cat` more friendly
        f.write(rpc.format_message([blob]) + "\n")

    return [{"status": "ok"}]

def handle_delete_everything(request):
    print("purging directory: " + storage_root)
    try:
        shutil.rmtree(storage_root)
    except OSError:
        pass
    return [{"status": "ok"}]

bad_keys = ("", ".", "..")

def get_storage_path(o, include_entry=True):
    dir = o.get("dir", "")
    if include_entry:
        entry = o.get("entry", "")
    else:
        entry = "dummy"

    if any(x in bad_keys for x in (dir, entry)):
        print("bad args")
        return None

    if any(re.search(r"[^A-Za-z0-9.-]", x) != None for x in (dir, entry)):
        print("illegal characters in dir/entry")
        return None

    if include_entry:
        return os.path.join(storage_root, dir, entry + ".txt")
    else:
        return os.path.join(storage_root, dir)

if __name__ == "__main__":
    main()
