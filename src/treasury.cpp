#include <treasury.hpp>

using namespace treasuryspace; 

treasury::treasury(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

treasury::~treasury() {}

// ensure that at least one active treasurer has signed this transaction
void treasury::check_treasurer_approval () {
	treas_table t_t (get_self(), get_self().value);
	auto t_itr = t_t.begin();
	while (t_itr != t_t.end()) {
		if (has_auth(t_itr->treasurer)) {
			return;
		}
		t_itr++;
	}
	check (false, "You must be a current treasurer to execute this action");
}

void treasury::addtreasrer(const name &treasurer, map<string, string> &notes) {
	require_auth (get_self());
	treas_table t_t (get_self(), get_self().value);
	auto t_itr = t_t.find (treasurer.value);
	check (t_itr == t_t.end(), "Account: " + treasurer.to_string() + " is already a treasurer");
	t_t.emplace (get_self(), [&](auto &t) {
		t.treasurer = treasurer;
		t.notes.insert(notes.begin(), notes.end());
		t.updated_date = current_time_point();
	});
}

void treasury::remtreasrer(const name &treasurer, const string &note) {
	require_auth (get_self());
	treas_table t_t (get_self(), get_self().value);
	auto t_itr = t_t.find (treasurer.value);
	check (t_itr != t_t.end(), "Account: " + treasurer.to_string() + " is not a treasurer");
	t_t.erase (t_itr);
}

// Set new treasurers after elections
void treasury::settreasrers(vector<name> &treasurers, map<string, string> &notes) {

	require_auth (get_self());

	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());	
	const uint32_t threshold = static_cast<uint32_t>(c.ints.at("threshold"));
	check (threshold > 0, "Threshold must be greater than 0. Threshold: " + std::to_string(threshold));
	check (threshold <= treasurers.size(), "Threshold must be less than or equal to the number of treasurers. Threshold: " +
		std::to_string(threshold) + "; number of treasurers attempting to set: " + std::to_string(treasurers.size()));

  	for (std::vector<name>::iterator it = treasurers.begin() ; it != treasurers.end(); ++it) {
		notes["system note"] = string ("added via settreasrers");
		addtreasrer (*it, notes);
	}
}