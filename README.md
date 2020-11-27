## Create a Matching Service for an online Dating Site
## Syetem Requirements

* Python 3.x or Python 2.7
* make
* a C++ compiler that supports -std=c++11
* The ability to bind to ports 8800 and 8801.

## Getting started

Start the storage server and leave it running forever:

```
./bin/storage-server.py
```

In a separate terminal, build and run the matching service.
This is the service that you will be maintaining:

```
make && ./build/matching-service
```

To test that it's working, run the test script:

```
./bin/test.py
```

To see a diagram of the interaction between these services, see `doc/architecture_diagram.png`:

## Overview

### API

See `doc/API.md`.

### RPC message structure

Every RPC call between processes in this codebase follows the same structure,
and encodes an array of objects that each encode key/value pairs.
The key and value are delimited by a colon.
Key/value pairs are delimited by newlines.
Objects of key/value pairs are delimited by double newlines.

Here is an example encoding 2 objects each with 2 key/value pairs:

```
key_1:value_1
key_2:value_2

key_1:value_1
key_2:value_2
```

This is isomorphic to a JSON array of objects with string values:

```
[
    {
        "key_1": "value_1",
        "key_2": "value_2"
    },
    {
        "key_1": "value_1",
        "key_2": "value_2"
    }
]
```

Each message must have at least 1 object, and each object must contain at least 1 key/value pair.
The keys and values are always strings.

### Storage structure

The storage engine stores blobs of data keyed by a "directory" name and an "entry" name.
The blob is stored in the file `data/<directory>/<entry>.txt`.
The contents of the file is a sequence of key/value pair lines
in the same format as a single object in the above-described RPC structure,
and then followed by a single newline to be more friendly to `cat`.

The "directory" is analogous to a table in SQL, and the "entry" is analogous to a primary key.
For example `data/user/alice.txt` would hold the information for the user named `alice`.

```
$ cat data/user/alice.txt
gender:female
```
