# API

All APIs can return a generic error, for example for an unsupported command or a malformed request.
This is communicated on the wire as 0 bytes returned,
and the function that reads the response will return false.

## matching-service

### `match_search`

Returns all the users who the specified user has not yet voted on.

request:

```
command:match_search
userid:<userid>
```

response, either:

```
error:none
```

or:

```
userid:<userid>

userid:<userid>

...
```

The userids returned will always be in alphabetical/lexicographical order.

## storage-server

NOTE: `rpc.hpp` defines utility methods for calling all these APIs from C++ code.

### `get`

Gets one or more records.

request:

```
command:get

dir:<dir>
entry:<entry>

dir:<dir>
entry:<entry>

...
```

response:

```
<response>

<response>

...
```

where each `<response>` is either:
* `error:<message>` such as:
  * `error:not_found` - no such entry is stored.
* or a sequence of key/value pairs representing the stored object.

There will always be one response per dir/entry pair in the request.

### `list_dir`

Lists the entries in a directory.

request:

```
command:list_dir
dir:<dir>
```

response, either:

```
error:none
```

indicating there are no entries in the specified directory, or:

```
entry:<entry>

entry:<entry>

...
```

The entries returned will always be sorted in alphabetical/lexicographical order.

### `put`

Writes the specified object to storage.

request:

```
command:put
dir:<dir>
entry:<entry>

<object>
```

response is always:

```
status:ok
```

### `delete_everything`

Blows away all stored data.

request:

```
command:delete_everything
```

response is always:

```
status:ok
```
