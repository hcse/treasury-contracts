#include <treasury.hpp>

// multisig
void treasury::setconfig(
					const map<string, name> names,
                    const map<string, string> strings,
                    const map<string, asset> assets,
                    const map<string, uint64_t> ints) {
	require_auth(get_self());



	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());
	
	c.assets = assets;
	c.names = names;
	c.strings = strings;
	c.ints = ints;
	c.updated_date	= current_time_point();

	config_s.set(c, get_self());

	// validate for required configurations
	string required_names[]{"token_redemption_contract", "dao_contract"};
	for (int i{0}; i < std::size(required_names); i++)
	{
		check(c.names.find(required_names[i]) != c.names.end(), "name configuration: " + required_names[i] + " is required but not provided.");
	}
}

void treasury::setredsymbol(const symbol& redemption_symbol) {
	require_auth(get_self());

	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());

	c. redemption_symbol 	= redemption_symbol;
	config_s.set(c, get_self());
}

// Set new treasurers after elections
void treasury::settreasrers(vector<name> &treasurers) {
	vector<permissions::permission_level_weight> accounts;

	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());	
	const uint32_t threshold = static_cast<uint32_t>(c.ints.at("threshold"));
	check (threshold > 0, "Threshold must be greater than 0. Threshold: " + std::to_string(threshold));
	check (threshold <= treasurers.size(), "Threshold must be less than or equal to the number of treasurers. Threshold: " +
		std::to_string(threshold) + "; number of treasurers attempting to set: " + std::to_string(treasurers.size()));

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

	permissions::authority auth = permissions::authority{threshold, {}, accounts, {}};
	auto update_auth_payload = std::make_tuple(get_self(), name("active"), name("owner"), auth);

	action(
        permission_level{get_self(), name("owner")},
        name("eosio"),
        name("updateauth"),
        update_auth_payload)
    .send();
}

// to be called by redeemer
void treasury::redeem(const name &redeemer, const asset &amount, const map<string, string> &notes){
	require_auth (redeemer);

	confirm_balance (redeemer, amount);

	// deduct from redeemer's balance
	balance_table balances(get_self(), redeemer.value);
	auto b_itr = balances.find(amount.symbol.code().raw());
	balances.modify(b_itr, get_self(), [&](auto& b){
		b.funds -= amount;
	});
	
	redemption_table r_t (get_self(), get_self().value);
	r_t.emplace (get_self(), [&](auto &r) {
		r.redemption_id		= r_t.available_primary_key();
		r.redeemer			= redeemer;
		r.amount			= amount;
		r.notes 			= notes;
	});
}

// add sink to a redemption request; covers any other data a user wishes to add
void treasury::addnotes (const uint64_t& redemption_id, const map<string, string> &notes){
	redemption_table r_t (get_self(), get_self().value);
	auto r_itr = r_t.find(redemption_id);
	check (r_itr != r_t.end(), "Redemption ID is not found: " + std::to_string(redemption_id));
	check (has_auth(r_itr->redeemer) || has_auth(get_self()), "Unauthorized. Only the redeemer or the treasury can add notes.");
	
	r_t.modify (r_itr, get_self(), [&](auto& r) {
		r.notes.insert(notes.begin(), notes.end());
	});
}

// multisig, existing treasurers, or eosio.code; to be called by treasurers, burns the tokens associated with the redemption
void treasury::paid(const uint64_t &redemption_id, const asset& amount, const map<string, string> &notes){
	require_auth (get_self());
	redemption_table r_t (get_self(), get_self().value);
	auto r_itr = r_t.find(redemption_id);
	check (r_itr != r_t.end(), "Redemption ID is not found: " + std::to_string(redemption_id));
	check (amount <= r_itr->amount, "Redemption amount must be less than requested amount. Requested amount: " +
		r_itr->amount.to_string() + "; Redeemed amount: " + amount.to_string());

	if (amount < r_itr->amount) {
		r_t.modify (r_itr, get_self(), [&](auto& r) {
			r.amount -= amount;
		});
	} else {
		r_t.erase (r_itr);
	}

	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());
	string burn_memo = string("Redemption ID: " + std::to_string(redemption_id));
	// burn the tokens
	action(
		permission_level{get_self(), name("active")},
		c.names.at("token_redemption_contract"), name("retire"),
		make_tuple(amount, burn_memo))
		.send();
}

// multisig, existing treasurers; calls redeemed for each redemption in the map; reduces number of multisigs needed
void treasury::redeemedmap(const map<uint64_t, string> redemptions){}

void treasury::deposit ( const name& from, const name& to, const asset& quantity, const string& memo ) {

   eosio::check (!is_paused(), "Contract is paused - no actions allowed.");
   if (to != get_self()) { return; }  // not sending to treasury

   config_table      config_s (get_self(), get_self().value);
   Config c = config_s.get_or_create (get_self(), Config());
   check (quantity.symbol == common::S_HUSD, "Only redemptions of HUSD are accepted.");
  
   check (get_first_receiver() == c.names.at("token_redemption_contract"),  
      "Only deposits of the configured HUSD token contract are accepted. Configured contract: " + 
	  c.names.at("token_redemption_contract").to_string() + "; you used: " + get_first_receiver().to_string());
  
   balance_table balances(get_self(), from.value);
   asset new_balance;
   auto it = balances.find(quantity.symbol.code().raw());
   if(it != balances.end()) {
      check (it->token_contract == get_first_receiver(), "Transfer does not match existing token contract.");
      balances.modify(it, get_self(), [&](auto& bal){
         // Assumption: total currency issued by eosio.token will not overflow asset
         bal.funds += quantity;
         new_balance = bal.funds;
      });
   }
   else {
      balances.emplace(get_self(), [&](auto& bal){
         bal.funds = quantity;
         bal.token_contract  = get_first_receiver();
         new_balance = quantity;
      });
   }

   print ("\n");
   print(name{from}, " deposited:       ", quantity, "\n");
   print(name{from}, " funds available: ", new_balance);
   print ("\n");
}

void treasury::pause()
{
	require_auth(get_self());
	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());
	c.ints["paused"] = 1;
	config_s.set(c, get_self());
}

void treasury::unpause()
{
	require_auth(get_self());
	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());
	c.ints["paused"] = 0;
	config_s.set(c, get_self());
}