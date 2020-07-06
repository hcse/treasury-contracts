#include <treasury.hpp>

using namespace treasuryspace;

// multisig
void treasury::setconfig(
    const map<string, name> names,
    const map<string, string> strings,
    const map<string, asset> assets,
    const map<string, uint64_t> ints)
{

    require_auth(get_self());

    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());

    c.assets = assets;
    c.names = names;
    c.strings = strings;
    c.ints = ints;
    c.updated_date = current_time_point();

    config_s.set(c, get_self());

    // validate for required configurations
    string required_names[]{"token_redemption_contract", "dao_contract"};
    for (int i{0}; i < std::size(required_names); i++)
    {
        check(c.names.find(required_names[i]) != c.names.end(), "name configuration: " + required_names[i] + " is required but not provided.");
    }
}

void treasury::setthreshold(const uint64_t &threshold) {
    require_auth(get_self());

    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());

    c.ints["threshold"] = threshold;
    config_s.set(c, get_self());
}

void treasury::setredsymbol(const symbol &redemption_symbol)
{
    require_auth(get_self());

    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());

    c.redemption_symbol = redemption_symbol;
    config_s.set(c, get_self());
}

void treasury::pauseredmp(const string &note)
{
    require_auth(get_self());
    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());
    c.ints["redemption_paused"] = 1;
    config_s.set(c, get_self());
}

void treasury::unpauseredmp(const string &note)
{
    require_auth(get_self());
    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());
    c.ints["redemption_paused"] = 0;
    config_s.set(c, get_self());
}

void treasury::pauseall(const string &note)
{
    require_auth(get_self());
    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());
    c.ints["paused"] = 1;
    config_s.set(c, get_self());
}

void treasury::unpauseall(const string &note)
{
    require_auth(get_self());
    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());
    c.ints["paused"] = 0;
    config_s.set(c, get_self());
}

bool treasury::is_paused()
{
    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());
    check(c.ints.find("paused") != c.ints.end(), "Contract does not have a pause configuration. Assuming it is paused. Please contact administrator.");

    uint64_t paused = c.ints.at("paused");
    return paused == 1;
}

bool treasury::is_redemption_paused()
{
    config_table config_s(get_self(), get_self().value);
    Config c = config_s.get_or_create(get_self(), Config());
    check(c.ints.find("redemption_paused") != c.ints.end(), "Contract does not have a redemption_paused configuration. Assuming it is paused. Please contact administrator.");

    uint64_t paused = c.ints.at("redemption_paused");
    return paused == 1;
}

void treasury::confirm_balance(const name &account, const asset &amount)
{

    balance_table balances(get_self(), account.value);
    auto b_itr = balances.find(amount.symbol.code().raw());
    check(b_itr != balances.end(), "Account has no balance for this symbol. account: " + account.to_string() +
                                       "; symbol: " + amount.symbol.code().to_string());
    check(b_itr->funds >= amount, "Insufficient funds. account: " + account.to_string() + " has balance of " +
                                      b_itr->funds.to_string() + ".  Transaction requires: " + amount.to_string());
}

// DEV ONLY
// void treasury::reset (name nothing) {
//     require_auth (get_self());
//     redemption_table r_t (get_self(), get_self().value);
//     auto r_itr = r_t.begin();
//     while (r_itr != r_t.end()) {
//         r_itr = r_t.erase (r_itr);
//     }

//     payment_table p_t (get_self(), get_self().value);
//     auto p_itr = p_t.begin();
//     while (p_itr != p_t.end()) {
//         p_itr = p_t.erase (p_itr);
//     }
// }
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
