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

// to be called by requestor
void treasury::redeem(const name &requestor, const asset &amount_requested, const map<string, string> &notes){

	eosio::check (!is_redemption_paused(), "Maintenance period. Redemptions are not available at this moment; please try again later.");
    eosio::check (!is_paused(), "Contract is paused - no actions allowed.");
	require_auth (requestor);
	
	confirm_balance (requestor, amount_requested);

	// deduct from requestor's balance
	balance_table balances(get_self(), requestor.value);
	auto b_itr = balances.find(amount_requested.symbol.code().raw());
	balances.modify(b_itr, get_self(), [&](auto& b){
		b.funds -= amount_requested;
	});
	
	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());

	redemption_table r_t (get_self(), get_self().value);
	r_t.emplace (get_self(), [&](auto &r) {
		r.redemption_id		= r_t.available_primary_key();
		r.requestor			= requestor;
		r.amount_requested	= amount_requested;
		r.amount_paid		= asset (0, c.redemption_symbol);
		r.notes 			= notes;
	});
}

void treasury::add_notes (const uint64_t& redemption_id, const map<string, string> &notes) {
	redemption_table r_t (get_self(), get_self().value);
	auto r_itr = r_t.find(redemption_id);
	check (r_itr != r_t.end(), "Redemption ID is not found: " + std::to_string(redemption_id));
	
	r_t.modify (r_itr, get_self(), [&](auto& r) {
		r.notes.insert(notes.begin(), notes.end());
	});
}

// this allows the redemption requestor to add notes to their request
void treasury::addnotesuser (const uint64_t& redemption_id, const map<string, string> &notes){
	redemption_table r_t (get_self(), get_self().value);
	auto r_itr = r_t.find(redemption_id);
	check (r_itr != r_t.end(), "Redemption ID is not found: " + std::to_string(redemption_id));
	check (has_auth(r_itr->requestor), "Unauthorized. Only the redeemer can add notes from this action.");
	add_notes (redemption_id, notes);
}

// permissioned to singletreas
// add sink to a redemption request; covers any other data a user wishes to add
void treasury::addnotestres (const uint64_t& redemption_id, const map<string, string> &notes){
	add_notes (redemption_id, notes);
}

// when payments are confirmed by a threshold of treasurers, the corresponding tokens are burned
void treasury::burn_tokens (const asset& amount, const string &burn_memo) {

	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());
	// burn the tokens
	action(
		permission_level{get_self(), name("active")},
		c.names.at("token_redemption_contract"), name("retire"),
		make_tuple(amount, burn_memo))
		.send();
}

// this is called when a threshold of treasurers attest to the payment
void treasury::confirm_payment (const uint64_t& redemption_id, const asset& amount) {
	redemption_table r_t (get_self(), get_self().value);
	auto r_itr = r_t.find(redemption_id);
	check (r_itr != r_t.end(), "Redemption ID is not found: " + std::to_string(redemption_id));
	check (amount <= r_itr->amount_requested, "Redemption amount must be less than requested amount. Requested amount: " +
		r_itr->amount_requested.to_string() + "; Redeemed amount: " + amount.to_string());

	r_t.modify (r_itr, get_self(), [&](auto& r) {
		r.amount_paid += amount;
	});	

	burn_tokens (amount, string("Redemption ID: " + std::to_string(redemption_id)));
}

// permissioned to singletreas
void treasury::attestpaymnt (const name& treasurer, const uint64_t& payment_id, 
							 const uint64_t& redemption_id, const asset& amount, const map<string, string> &notes) {

	require_auth (treasurer);
	payment_table p_t (get_self(), get_self().value);
	auto p_itr = p_t.find (payment_id);

	check (p_itr->redemption_id == redemption_id, "redemption_id does not match data on payment you are attesting to. You provided: " +
		std::to_string(redemption_id) + "; the payment record being attested has: " + std::to_string(p_itr->redemption_id));
	check (p_itr->amount_paid == amount, "amount does not match amount on payment you are attesting to. You provided: " +
		amount.to_string() + "; the payment record being attested has: " + p_itr->amount_paid.to_string());

	std::map<name, time_point>::const_iterator itr;
	for (itr = p_itr->attestations.begin(); itr != p_itr->attestations.end(); ++itr) {
		check (itr->first != treasurer, "Treasurer: " + treasurer.to_string() + " has already attested to this payment.");
	}

	config_table      config_s (get_self(), get_self().value);
   	Config c = config_s.get_or_create (get_self(), Config());

	p_t.modify (p_itr, get_self(), [&](auto &p) {
		p.payment_id = p_t.available_primary_key();
		p.creator = treasurer;
		p.redemption_id	= redemption_id;
		p.amount_paid = amount;

		// this merge doesn't compile, so doing it manually in for loop below
		// p.notes = notes.merge(p_itr->notes);	// new notes being passed gets precedent

		// merge all new notes, over-writing any notes with the same key
		std::map<string, string>::const_iterator note_itr;
		for (note_itr = notes.begin(); note_itr != notes.end(); ++note_itr) {
			p.notes[note_itr->first] = note_itr->second;
		}

		p.attestations[treasurer] = current_time_point();
		if (p_itr->attestations.size() >= c.ints.at("threshold")) {  // we have enough attestations to clear the redemptions request
			confirm_payment (p_itr->redemption_id, p_itr->amount_paid); 
			p.confirmed_date = current_time_point();
		}
	});
}

void treasury::newpayment(const name& treasurer, const uint64_t &redemption_id, const asset& amount, const map<string, string> &notes){
	require_auth (treasurer);
	redemption_table r_t (get_self(), get_self().value);
	auto r_itr = r_t.find(redemption_id);
	check (r_itr != r_t.end(), "Redemption ID is not found: " + std::to_string(redemption_id));
	
	asset amount_due = r_itr->amount_requested - r_itr->amount_paid;
	check (amount <= amount_due, "Redemption amount must be less than amount due. Original requested amount: " +
		r_itr->amount_requested.to_string() + "; Paid amount: " + r_itr->amount_paid.to_string() + ". The remaining amount due is: " +
		amount_due.to_string() + " and you attempted to create a new payment for: " + amount.to_string());
	
	payment_table p_t (get_self(), get_self().value);
	p_t.emplace (get_self(), [&](auto &p) {
		p.payment_id = p_t.available_primary_key();
		p.creator = treasurer;
		p.redemption_id	= redemption_id;
		p.amount_paid = amount;
		p.notes = notes;
		p.attestations[treasurer] = current_time_point();
	});
}

// void treasury::backupreds () {
// 	require_auth (get_self());
// 	oredemption_table r_t (get_self(), get_self().value);
// 	backup_redemption_table bu_t (get_self(), get_self().value);
// 	auto r_itr = r_t.begin();

// 	while (r_itr != r_t.end()) {
// 		bu_t.emplace (get_self(), [&](auto &r) {
// 			r.redemption_id = r_itr->redemption_id;
// 			r.redeemer = r_itr->redeemer;
// 			r.amount = r_itr->amount;
// 			r.notes = r_itr->notes;
// 		});
// 		r_itr = r_t.erase (r_itr);
// 	}
// }

void treasury::deposit ( const name& from, const name& to, const asset& quantity, const string& memo ) {

   if (to != get_self()) { return; }  // not sending to treasury

   eosio::check (!is_paused(), "Contract is paused - no actions allowed.");
   eosio::check (!is_redemption_paused(), "Maintenance period. Redemptions are not available at this moment; please try again later.");

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

void treasury::pauseredmp()
{
	require_auth(get_self());
	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());
	c.ints["redemption_paused"] = 1;
	config_s.set(c, get_self());
}

void treasury::unpauseredmp()
{
	require_auth(get_self());
	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());
	c.ints["redemption_paused"] = 0;
	config_s.set(c, get_self());
}

void treasury::pauseall()
{
	require_auth(get_self());
	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());
	c.ints["paused"] = 1;
	config_s.set(c, get_self());
}

void treasury::unpauseall()
{
	require_auth(get_self());
	config_table config_s(get_self(), get_self().value);
	Config c = config_s.get_or_create(get_self(), Config());
	c.ints["paused"] = 0;
	config_s.set(c, get_self());
}