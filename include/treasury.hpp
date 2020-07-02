#include <algorithm>

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

#include "permissions.hpp"

using namespace eosio;
using std::map;
using std::variant;
using std::vector;

namespace treasuryspace
{

   CONTRACT treasury : public contract
   {
   public:
      treasury(name self, name code, datastream<const char*> ds);
      ~treasury();

      struct [[eosio::table, eosio::contract("treasury")]] Config
      {
         // required configurations:
         // names    :  dao_contract, seeds_token_contract
         // ints     :  threshold
         symbol redemption_symbol;
         map<string, name> names;
         map<string, string> strings;
         map<string, asset> assets;
         map<string, uint64_t> ints;
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
         uint64_t redemption_id;
         name requestor;
         asset amount_requested;
         asset amount_paid;
         map<string, string> notes; // notes should contain the network of the redemption plus the trx ID

         time_point requested_date = current_time_point();
         time_point updated_date = current_time_point();

         uint64_t primary_key() const { return redemption_id; }
         uint64_t by_requestor() const { return requestor.value; }
         uint64_t by_amount_due() const { return amount_requested.amount - amount_paid.amount; }
         uint64_t by_requested_date() const { return requested_date.sec_since_epoch(); }
         uint64_t by_updated_date() const { return updated_date.sec_since_epoch(); }
      };

      typedef multi_index<name("redemptions"), Redemption,
                          indexed_by<name("byrequestor"), const_mem_fun<Redemption, uint64_t, &Redemption::by_requestor>>,      // #2
                          indexed_by<name("byamountdue"), const_mem_fun<Redemption, uint64_t, &Redemption::by_amount_due>>,     // #3
                          indexed_by<name("byrequestdt"), const_mem_fun<Redemption, uint64_t, &Redemption::by_requested_date>>, // #4
                          indexed_by<name("byupdateddt"), const_mem_fun<Redemption, uint64_t, &Redemption::by_requested_date>>  // #5
                          >
          redemption_table;

      struct [[eosio::table, eosio::contract("treasury")]] RedemptionPayment
      {
         uint64_t payment_id;    // incrementer
         name creator;           // creator of this payment, must be a treasurer
         uint64_t redemption_id; // foreign key to the redemptions table
         asset amount_paid;      // should always be less than or equal to the requested amount on the redemption

         time_point created_date = current_time_point(); // date that the redemption was created
         time_point confirmed_date;                      // date that the redemption was completed

         map<name, time_point> attestations; // accounts attesting that this payment was made
         map<string, string> notes;          // should always contain the network and transaction ID of the redemption

         uint64_t primary_key() const { return payment_id; }
         uint64_t by_redemption() const { return redemption_id; }
         uint64_t by_created_date() const { return created_date.sec_since_epoch(); }
         uint64_t by_confirmed_date() const { return confirmed_date.sec_since_epoch(); }
      };

      typedef multi_index<name("payments"), RedemptionPayment,
                          indexed_by<name("byredemption"), const_mem_fun<RedemptionPayment, uint64_t, &RedemptionPayment::by_redemption>>,    // #2
                          indexed_by<name("bycreated"), const_mem_fun<RedemptionPayment, uint64_t, &RedemptionPayment::by_created_date>>,     // #3
                          indexed_by<name("bycompletedt"), const_mem_fun<RedemptionPayment, uint64_t, &RedemptionPayment::by_confirmed_date>> // #4
                          >
          payment_table;

      [[eosio::on_notify("*::transfer")]] void deposit(const name &from, const name &to, const asset &quantity, const string &memo);
      using transfer_action = action_wrapper<name("transfer"), &treasury::deposit>;

      // ---------------- ADMIN / SETUP -------------------------
      // ADMIN / setup actions / can be removed after setup
      // multisig
      ACTION setconfig(const map<string, name> names,
                       const map<string, string> strings,
                       const map<string, asset> assets,
                       const map<string, uint64_t> ints);

      ACTION pauseredmp(const string &note);
      ACTION unpauseredmp(const string &note);
      ACTION pauseall(const string &note);
      ACTION unpauseall(const string & note);
      ACTION setredsymbol(const symbol &redemption_symbol);

      ACTION settreasrers(const vector<name> & treasurers);

      // DEV ONLY
      // ACTION reset (name nothing);
      // -----------------------------------------------------0--

      // to be called by redeemer
      ACTION redeem(const name &redeemer, const asset &amount, const map<string, string> &notes);

      // add sink to a redemption request; covers any other data a user wishes to add
      ACTION addnotesuser(const uint64_t &redemption_id, const map<string, string> &notes);
      ACTION addnotestres(const uint64_t &redemption_id, const map<string, string> &notes);

      // treasurer attests that the redemption was paid to the requestor
      ACTION attestpaymnt(const name &treasurer, const uint64_t &payment_id,
                          const uint64_t &redemption_id, const asset &amount, const map<string, string> &notes);

      ACTION removeattest(const name &treasurer, const uint64_t &payment_id, const map<string, string> &notes);

      // when a treasurer issues a payment, s/he should also create the payment on the Telos chain
      ACTION newpayment(const name &treasurer, const uint64_t &redemption_id, const asset &amount, const map<string, string> &notes);

   private:
      permissions::authority get_treasurer_authority(const vector<name> & treasurers, const uint16_t &threshold);
      void confirm_payment(const uint64_t &redemption_id, const asset &amount);
      void burn_tokens(const asset &amount, const string &burn_memo);
      void add_notes(const uint64_t &redemption_id, const map<string, string> &notes);
      bool is_paused();
      bool is_redemption_paused();
      void confirm_balance(const name &account, const asset &amount);
   };

} // namespace treasuryspace