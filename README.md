> NOTE: Operators use at their own risk. Developers do not accept any risks associated with faulty code or regulatory concerns. Software is written for research purposes only. Not for use in production. Use only with tobacco. Do not try this at home. Disclaimer to cover all possible types of disclaimers.

# Hypha Treasury
### Design
#### Redemption Lifecycle
1. Treasury and DAO account(s) have permission to issue and retire HUSD
2. DAO issues HUSD based on proposals that pass
3. HUSD holders send HUSD to the treasury and create a redemption record
4. Treasurers transfer appropriate tokens (can be off-chain, a different chain, or avocados, etc). 
5. Treasurers issue a transaction confirming redemption, which removes the redemption record and retires (burns) the HUSD tokens 

#### Electing New Treasurers
1. Electing new treasurers should be handled by the DAO
2. It should be a rolling election of sorts where all treasurers are not replaced at once, perhaps one every 2-3 months
3. When a treasurer is replaced, the existing treasurers must call "settreasrers" with the new set of treasurers
```
ACTION settreasrers(const vector<name> &treasurers);
```

## Configuration
Configuration requires:
- Symbol that will redeemable (HUSD)
- Token contract for the redeemable token
- Threshold for the number of treasurers required to approve a redemption

## Action Sequence

#### Deposit
Registers a balance for the redeemer. This is automatically triggered when a user transfer to the contract.
```
void deposit(const name &from, const name &to, const asset &quantity, const string &memo);
```

#### Create Redemption Record
```
ACTION redeem(const name &redeemer, const asset &amount, const map<string, string> &notes);
```

#### Off-chain Redemption Occurs
< .. no action within this contract necessary .. >

#### Confirm Redemption
Erases the redemption record and burns the tokens
```
ACTION redeemed(const uint64_t &redemption_id, const asset& amount, const map<string, string> &notes);
```

### Notes
Redeemers or the treasury can add notes to the redemption records. This should be used to communicate any non-confidential information about the process so that it is transparent on chain.