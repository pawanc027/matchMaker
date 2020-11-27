#include "matching-handler.hpp"

#include <assert.h>

#include <iostream>

using namespace std;

static bool handle_match_search(const message & request, message & response) {
	if (request.size() != 1) {
		cerr << "bad args\n";
		return false;
	}
	string searcher = get_or_default(request[0], "userid", "");
	string limit = get_or_default(request[0], "limit", "3");

	std::cout<<"searcher:"<<searcher<<std::endl;
	std::cout<<"limit:"<<limit<<std::endl;

	// Start with the set of all users, then filter it down.
	vector<string> all_userids;
	if (!do_storage_list_dir("user", all_userids)) {
		cerr << "unable to list userids\n";
		return false;
	}
	// Userids are in alphabetical order.

	// Filter the set down
	for (string other_userid : all_userids) {
		if (other_userid == searcher) {
			// don't show you to yourself
			continue;
		}

		// check if we've already voted on them
		string vote_key = searcher + "-" + other_userid;
		string_map vote_object;
		if (!do_storage_get("vote", vote_key, vote_object)) {
			cerr << "unable to check vote\n";
			return false;
		}

		int vote_value = get_or_default(vote_object, "vote", -1);
		/*
		std::cout<<"vote_value:"<<vote_value<<std::endl;
		std::cout<<"other_userid:"<<other_userid<<std::endl;
		*/
		if (vote_value == -1 ) {
			// check if we've already voted on them
			string vote_key = other_userid + "-" + searcher;
			string_map vote_object;
			if (!do_storage_get("vote", vote_key, vote_object)) {
				cerr << "unable to check vote\n";
			}

			int vote_value = get_or_default(vote_object, "vote", -1);
			/*
			std::cout<<"vote_value:"<<vote_value<<std::endl;
			std::cout<<"other_userid:"<<other_userid<<std::endl;
			*/
			if (vote_value == -1 || vote_value > std::stoi(limit) ) {
				// hasn't voted on this person yet.
				response.push_back({
						{"userid", other_userid},
						});
			}
		}
	}

	for( const auto& entry:response)
	{
		for(auto itr=entry.begin();itr!= entry.end();++itr)
			std::cout<<itr->first<<" "<<itr->second<<"\n";
	}

	if (response.size() == 0) {
		response.push_back({
				{"error", "none"},
				});
	}



	return true;
}

bool handle_request(const message & request, message & response) {
	assert(request.size() >= 1);
	string command = get_or_default(request[0], "command", "");
	if (command == "match_search") {
		return handle_match_search(request, response);
	}
	cerr << "ERROR: unsupported command: " << command << "\n";
	return false;
}
