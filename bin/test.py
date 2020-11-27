#!/usr/bin/env python

import rpc
import sys

def main():
    # to see a diagram of the vote interaction between the test users, see doc/vote_diagram.png
    setup_test()

    #do_tests()
    do_tests_assignment1()

def do_tests():
    # alice has already voted on everyone
    test_match_search("alice", [])

    # bob has only voted on alice
    test_match_search("bob", ["chris"])

    # chris hasn't voted on anyone
    test_match_search("chris", ["alice", "bob"])

    print("all tests pass")

def do_tests_assignment1():
    # alice has already voted on everyone
    test_match_search("alice", [])

    # bob has only voted on alice
    test_match_search("bob", ["chris"])

    # chris hasn't voted on anyone, but alice voted poorly for chris
    test_match_search("chris", ["bob"])

    print("all tests pass")


def test_match_search(searcher, expected_users):
    response = rpc.do_api(rpc.matching_server, [{
        "command": "match_search",
        "userid": searcher,
        #"limit": 2,
    }])
    if response == None:
        assert False, "got an error"
    if response == [{"error": "none"}]:
        got_users = []
    else:
        got_users = [o.get("userid", "") for o in response]

    # the order of entries is guaranteed to be alphabetical.
    if got_users == expected_users:
        print("test_match_search for {}: PASS\nexpected/got: {}\n".format(searcher, repr(expected_users)))
        return

    # failure
    print("test_match_search for {}: FAILED\nexpected: {}\ngot: {}".format(searcher, repr(expected_users), repr(got_users)))
    sys.exit("!!!!!!!!!!!!!!!!!!!!")

def setup_test():
    requests = [
        [{"command": "delete_everything"}],
        # users
        make_put_command("user", "alice", gender="female"),
        make_put_command("user", "bob", gender="male"),
        make_put_command("user", "chris", gender="other"),
        # votes
        make_put_command("vote", "alice-bob", vote="1"),
        make_put_command("vote", "alice-chris", vote="3"),
        make_put_command("vote", "bob-alice", vote="5"),
        # (no votes for bob-chris, chris-alice, chris-bob)
    ]
    for request in requests:
        rpc.do_api(rpc.storage_server, request, verbose=False)

def make_put_command(dir, entry, **kwargs):
    return [
        {"command": "put", "dir": dir, "entry": entry},
        kwargs,
    ]

if __name__ == "__main__":
    main()
