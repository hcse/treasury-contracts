#include <treasury.hpp>

using namespace treasuryspace; 

treasury::treasury(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

treasury::~treasury() {}

permissions::authority treasury::get_treasurer_authority (vector<name> &treasurers, const uint16_t &threshold)  {
	vector<permissions::permission_level_weight> accounts;

	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());	

	// DAO contract issues the treasury token, so its permission must always be equal to the threshold
	check (c.names.find("dao_contract") != c.names.end(), "dao_contract configuration is required when updating permissions.");
	const name dao_contract = c.names.at("dao_contract");
	treasurers.push_back(dao_contract);
	treasurers.push_back(get_self());

	std::sort(treasurers.begin(), treasurers.end());	// accounts must be sorted

	for(name treasurer : treasurers) {
		uint16_t treas_threshold = 1;
		name permission_name = name("active");
		if (treasurer == dao_contract || treasurer == get_self()) {
			treas_threshold = static_cast<uint16_t>(threshold);
			permission_name = name("eosio.code");
		}
		permission_level pl = permission_level{treasurer, permission_name};
		permissions::permission_level_weight plw = permissions::permission_level_weight {pl, treas_threshold};
		accounts.push_back(plw);
	} 

	return permissions::authority{threshold, {}, accounts, {}};
}

// Set new treasurers after elections
void treasury::settreasrers(vector<name> &treasurers) {

	require_auth (get_self());

	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());	
	const uint32_t threshold = static_cast<uint32_t>(c.ints.at("threshold"));
	check (threshold > 0, "Threshold must be greater than 0. Threshold: " + std::to_string(threshold));
	check (threshold <= treasurers.size(), "Threshold must be less than or equal to the number of treasurers. Threshold: " +
		std::to_string(threshold) + "; number of treasurers attempting to set: " + std::to_string(treasurers.size()));

	// DAO contract issues the treasury token, so its permission must always be equal to the threshold
	check (c.names.find("dao_contract") != c.names.end(), "dao_contract configuration is required when updating permissions.");

	permissions::authority threshold_auth = get_treasurer_authority(treasurers, threshold);
	permissions::authority singletreas_auth = get_treasurer_authority(treasurers, 1);

	auto threshold_auth_payload = std::make_tuple(get_self(), name("active"), name("owner"), threshold_auth);
	auto singletreas_auth_payload = std::make_tuple(get_self(), name("singletreas"), name("active"), singletreas_auth);

	auto single_treas_action = action(permission_level{get_self(), name("owner")}, name("eosio"), name("updateauth"), singletreas_auth_payload);
    auto threshold_action = action(permission_level{get_self(), name("owner")}, name("eosio"), name("updateauth"), threshold_auth_payload);

	single_treas_action.send();
	threshold_action.send();
}