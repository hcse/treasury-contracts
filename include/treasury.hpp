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
      // names    :  dao_contract, seeds_token_contract
      // ints     :  threshold
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
      name                    requestor;
      asset                   amount_requested;
      asset                   amount_paid;
      map<string, string>     notes;         // notes should contain the network of the redemption plus the trx ID
      
      time_point requested_date = current_time_point();
      time_point updated_date = current_time_point();

      uint64_t primary_key() const { return redemption_id; }
      uint64_t by_requestor() const { return requestor.value; }
      uint64_t by_requested_date() const { return requested_date.sec_since_epoch(); }
      uint64_t by_updated_date() const { return updated_date.sec_since_epoch(); }
      uint64_t by_amount_due() const { return amount_requested.amount - amount_paid.amount; }
   };

   typedef multi_index<name("redemptions"), Redemption,
      indexed_by<name("byrequestor"), 
         const_mem_fun<Redemption, uint64_t, &Redemption::by_requestor>>
   > redemption_table;

   struct [[eosio::table, eosio::contract("treasury")]] RedemptionPayment
   {
      uint64_t                payment_id;               // incrementer
      name                    creator;             // creator of this payment, must be a treasurer
      uint64_t                redemption_id;       // foreign key to the redemptions table
      asset                   amount_paid;         // should always be less than or equal to the requested amount on the redemption

      time_point              created_date  = current_time_point();      // date that the redemption was completed
      time_point              confirmed_date;      // date that the redemption was completed

      map<name, time_point>   attestations;        // accounts attesting that this payment was made
      map<string, string>     notes;               // should always contain the network and transaction ID of the redemption
      
      uint64_t primary_key() const { return payment_id; }
      uint64_t by_redemption() const { return redemption_id; }
      uint64_t by_created_date() const { return created_date.sec_since_epoch(); }
      uint64_t by_confirmed_date() const { return confirmed_date.sec_since_epoch(); }
   };

   typedef multi_index<name("payments"), RedemptionPayment,
      indexed_by<name("byredemption"), 
         const_mem_fun<RedemptionPayment, uint64_t, &RedemptionPayment::by_redemption>>,
      indexed_by<name("bycreated"),
         const_mem_fun<RedemptionPayment, uint64_t, &RedemptionPayment::by_created_date>>,
      indexed_by<name("bycompletedt"),
         const_mem_fun<RedemptionPayment, uint64_t, &RedemptionPayment::by_confirmed_date>>
   > payment_table;

   [[eosio::on_notify("*::transfer")]] 
   void deposit(const name &from, const name &to, const asset &quantity, const string &memo);
   using transfer_action = action_wrapper<name("transfer"), &treasury::deposit>;

   // ---------------- ADMIN / SETUP -------------------------
   // multisig
   ACTION setconfig(const map<string, name> names,
                    const map<string, string> strings,
                    const map<string, asset> assets,
                    const map<string, uint64_t> ints);

   ACTION pauseredmp ();
   ACTION unpauseredmp ();
   ACTION pauseall ();
   ACTION unpauseall ();
   ACTION setredsymbol(const symbol& redemption_symbol);
   // ACTION backupreds ();

   // ADMIN / setup actions / can be removed after setup
   ACTION settreasrers(vector<name> &treasurers);
   // -----------------------------------------------------0-- 

   // to be called by redeemer
   ACTION redeem(const name &requestor, const asset &amount, const map<string, string> &notes);

   // add sink to a redemption request; covers any other data a user wishes to add
   ACTION addnotesuser (const uint64_t& redemption_id, const map<string, string> &notes);
   ACTION addnotestres (const uint64_t& redemption_id, const map<string, string> &notes);

   // treasurer attests that the redemption was paid to the requestor
   ACTION attestpaymnt (const name& treasurer, const uint64_t& payment_id, 
							 const uint64_t& redemption_id, const asset& amount, const map<string, string> &notes);

   // when a treasurer issues a payment, s/he should also create the payment on the Telos chain
   ACTION newpayment(const name& treasurer, const uint64_t &redemption_id, const asset& amount, const map<string, string> &notes);

private: 

   permissions::authority get_treasurer_authority (vector<name> &treasurers, const uint16_t &threshold);
   void confirm_payment (const uint64_t& redemption_id, const asset& amount);
   void burn_tokens (const asset& amount, const string &burn_memo);
   void add_notes (const uint64_t& redemption_id, const map<string, string> &notes) ;
   // void setowner (const string& pubk);

   bool is_paused () {
      config_table      config_s (get_self(), get_self().value);
      Config c = config_s.get_or_create (get_self(), Config());   
      check (c.ints.find ("paused") != c.ints.end(), "Contract does not have a pause configuration. Assuming it is paused. Please contact administrator.");
         
      uint64_t paused = c.ints.at("paused");
      return paused == 1;
   }   

   bool is_redemption_paused () {
      config_table      config_s (get_self(), get_self().value);
      Config c = config_s.get_or_create (get_self(), Config());   
      check (c.ints.find ("redemption_paused") != c.ints.end(), "Contract does not have a redemption_paused configuration. Assuming it is paused. Please contact administrator.");
         
      uint64_t paused = c.ints.at("redemption_paused");
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