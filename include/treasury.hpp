#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>

#include <eosio/action.hpp>
#include <eosio/contract.hpp>
#include <eosio/crypto.hpp>
#include <eosio/fixed_bytes.hpp>
#include <eosio/ignore.hpp>
#include <eosio/print.hpp>
#include <eosio/privileged.hpp>
#include <eosio/producer_schedule.hpp>

#include "common.hpp"
#include "permissions.hpp"

using namespace eosio;
using std::map;
using std::vector;
using std::variant;

CONTRACT treasury : public contract
{
public:
   using contract::contract;

   struct [[eosio::table, eosio::contract("treasury")]] Config
   {
      // required configurations:
      // names    :  husd_token_contract, seeds_token_contract
      // ints     :
      // assets   :  redemption_fee - just to discourage too many redemptions; paid back to DAO
      symbol                  redemption_symbol;
      map<string, name>       names;
      map<string, string>     strings;
      map<string, asset>      assets;
      map<string, uint64_t>   ints;

      // map<string, time_point> time_points;
      // map<string, transaction> trxs;
      // map<string, float> floats;
      // map<string, variant<name, string, asset, uint64_t>> variants;
      time_point updated_date = current_time_point();
   };

   typedef singleton<name("config"), Config> config_table;
   typedef multi_index<name("config"), Config> config_table_placeholder;

   struct [[eosio::table, eosio::contract("treasury")]] Balance
   {
      asset funds;
      name token_contract;
      uint64_t primary_key() const { return funds.symbol.code().raw(); }
   };

   typedef multi_index<name("balances"), Balance> balance_table;

   struct [[eosio::table, eosio::contract("treasury")]] Redemption 
   {
      uint64_t                redemption_id;
      name                    redeemer;
      asset                   amount;
      map<string, string>     notes;
      uint64_t primary_key() const { return redemption_id; }
   };

   typedef multi_index<name("redemptions"), Redemption> redemption_table;

   // workaround for bug on wildcard notification
   // https://github.com/EOSIO/eosio.cdt/issues/497#issuecomment-484691582
   // [[eosio::on_notify("dummy::dummy")]]
   // void dummy ();
   // using dummy_action = eosio::action_wrapper<"dummy"_n, &swaps::dummy>;

   [[eosio::on_notify("*::transfer")]] 
   void deposit(const name &from, const name &to, const asset &quantity, const string &memo);
   using transfer_action = action_wrapper<name("transfer"), &treasury::deposit>;

   // multisig
   ACTION setconfig(const map<string, name> names,
                    const map<string, string> strings,
                    const map<string, asset> assets,
                    const map<string, uint64_t> ints);

   // ACTION setconfig(const map<string, variant<name, string, asset, uint64_t>> my_values) ;
   ACTION pause ();
   ACTION unpause ();
   ACTION setredsymbol(const symbol& redemption_symbol);

   // ADMIN / setup actions / can be removed after setup
   ACTION settreasrers(vector<name> &treasurers);

   // multisig, existing treasurers; swaps existing treasurer for a new treasurer, e.g. after an election
   // ACTION swaptreasuer(const name &old_treasurer, const name &new_treasurer, const string &notes);

   // to be called by redeemer
   ACTION redeem(const name &redeemer, const asset &amount, const map<string, string> &notes);

   // add sink to a redemption request; covers any other data a user wishes to add
   ACTION addnotes (const uint64_t& redemption_id, const map<string, string> &notes);

   // multisig, existing treasurers, or eosio.code; to be called by treasurers, burns the tokens associated with the redemption
   ACTION paid(const uint64_t &redemption_id, const asset& amount, const map<string, string> &notes);

   // multisig, existing treasurers; calls redeemed for each redemption in the map; reduces number of multisigs needed
   ACTION redeemedmap(const map<uint64_t, string> redemptions);

   bool is_paused () {
      config_table      config_s (get_self(), get_self().value);
      Config c = config_s.get_or_create (get_self(), Config());   
      check (c.ints.find ("paused") != c.ints.end(), "Contract does not have a pause configuration. Assuming it is paused. Please contact administrator.");
         
      uint64_t paused = c.ints.at("paused");
      return paused == 1;
   }   

   void confirm_balance (const name& account, const asset& amount) {

      balance_table balances(get_self(), account.value);
      auto b_itr = balances.find(amount.symbol.code().raw());
      check (b_itr != balances.end(), "Account has no balance for this symbol. account: " + account.to_string() +
         "; symbol: " + amount.symbol.code().to_string());
      check (b_itr->funds >= amount, "Insufficient funds. account: " + account.to_string() + " has balance of " +
         b_itr->funds.to_string() + ".  Transaction requires: " + amount.to_string());
   }
};