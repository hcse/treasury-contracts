/* eslint-disable prettier/prettier */
const assert = require('assert');
const eoslime = require('eoslime').init('local');

const TREASURY_WASM_PATH = '../treasury/treasury.wasm';
const TREASURY_ABI_PATH = '../treasury/treasury.abi';
const EOSIOTOKEN_WASM_PATH = '/Users/max/dev/token/token/token.wasm';
const EOSIOTOKEN_ABI_PATH = '/Users/max/dev/token/token/token.abi';
 
async function getBalance (treasury, redeemer) {
    const balances = await treasury.provider.eos.getTableRows({
        code: treasury.name,
        scope: redeemer.name,
        table: 'balances',
        json: true
    });
    if (balances.rows.length == 0) {
        return "0.00 HUSD";
    }
    return balances.rows[0].funds;
}

async function assertTreasuryBalance (treasury, redeemer, balance) {
    const actualBalance = await getBalance (treasury, redeemer);
    assert.equal (actualBalance, balance);
}

describe('Treasury Testing', function () {

    // Increase mocha(testing framework) time, otherwise tests fails
    this.timeout(150000);

    let treasuryContract, eosTokenContract;
    let treasuryAccount, eosTokenAccount;
    let treasurer1, treasurer2, treasurer3, treasurer4, treasurer5;
    let redeemer1, redeemer2, redeemer3, redeemer4, redeemer5;
    let accounts;
    let config;
    
    before(async () => {

        accounts = await eoslime.Account.createRandoms(20);
        treasuryAccount     = accounts[0];
        eosTokenAccount     = accounts[1];
        daoAccount          = accounts[14];
        
        treasurer1          = accounts[4];
        treasurer2          = accounts[5];
        treasurer3          = accounts[6];
        treasurer4          = accounts[7];
        treasurer5          = accounts[8];
        redeemer1           = accounts[9];
        redeemer2           = accounts[10];
        redeemer3           = accounts[11];
        redeemer4           = accounts[12];
        redeemer5           = accounts[13];

        console.log (" Treasury Account     : ", treasuryAccount.name);
        console.log (" HUSD Issuer          : ", eosTokenAccount.name);
        console.log (" DAO Contract         : ", daoAccount.name);
        console.log (" treasurer1           : ", treasurer1.name);
        console.log (" treasurer2           : ", treasurer2.name);
        console.log (" treasurer3           : ", treasurer3.name);
        console.log (" treasurer4           : ", treasurer4.name);
        console.log (" treasurer5           : ", treasurer5.name);
        console.log (" redeemer1            : ", redeemer1.name);
        console.log (" redeemer2            : ", redeemer2.name);
        console.log (" redeemer3            : ", redeemer3.name);

        const initialBalance = '1000.00 HUSD';

        treasuryContract = await eoslime.Contract.deployOnAccount (TREASURY_WASM_PATH, TREASURY_ABI_PATH, treasuryAccount);
        eosTokenContract = await eoslime.Contract.deployOnAccount (EOSIOTOKEN_WASM_PATH, EOSIOTOKEN_ABI_PATH, eosTokenAccount);
        console.log ("  Treasury Contract   : ", treasuryContract.name);
        console.log ("  HUSD Token Contract : ", eosTokenContract.name);

        await treasuryAccount.addPermission('eosio.code');
        var treasuryOwner = await eoslime.Account.load(treasuryAccount.name, treasuryAccount.privateKey, 'owner');
        await treasuryOwner.addPermission('eosio.code');

        // const authorityInfo = await treasuryAccount.getAuthorityInfo();
        // authorityInfo.required_auth.accounts.push({ permission: { actor: treasuryAccount, permission: "eosio.code" }, weight: 1 });
        // await treasuryAccount.updateAuthority(this, "owner", "", authorityInfo.required_auth);

        // await treasuryAccount.provider.eos.transaction(tr => {
        //     tr.updateauth({
        //         account: this.name,
        //         permission: "owner",
        //         parent: "",
        //         auth: { permission: { actor: treasuryAccount, permission: "eosio.code" }, weight: 1 }
        //     }, { authorization: [this.executiveAuthority] });
    
        // }, { broadcast: true, sign: true, keyProvider: this.privateKey });
            
        await treasuryContract.setconfig([{"key":"token_redemption_contract","value":eosTokenContract.name},{"key":"dao_contract","value":daoAccount.name}], [], [], 
            [{"key":"paused","value":0}, {"key":"threshold","value":2}], { from: treasuryAccount});
        // await treasuryContract.togglepause ( {from: treasuryAccount} );
        await treasuryContract.setredsymbol("2,HUSD", { from: treasuryAccount });

        await eosTokenContract.create(treasuryAccount.name, '-1.00 HUSD', { from: eosTokenAccount });

        console.log ("\n\n");
        console.log (" Redeemers each receive ", initialBalance , " at the beginning of all tests.")
       
        await eosTokenContract.issue(treasuryAccount.name, '100000.00 HUSD', 'memo', { from: treasuryAccount});
        await eosTokenContract.transfer(treasuryAccount.name, redeemer1.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer2.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer3.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer4.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer5.name, initialBalance, 'memo', { from: treasuryAccount });

        console.log ("\nTreasury Configuration -----------------------")
        let configTable = await treasuryContract.provider.eos.getTableRows({
            code: treasuryContract.name,
            scope: treasuryContract.name,
            table: 'config',
            json: true
        });
        config = configTable.rows[0];
        console.log (config, "\n\n");
    });

    beforeEach(async () => {
   
    });

    it('Should create a redemption', async () => {
        
        console.log (" Redeemer ", redeemer1.name, " transfers 10.00 HUSD to Treasury account: ", treasuryAccount.name)
        await eosTokenContract.transfer (redeemer1.name, treasuryAccount.name, '10.00 HUSD', "memo", { from: redeemer1 });

        // check that the redeemer saved a balance
        await assertTreasuryBalance(treasuryContract, redeemer1, "10.00 HUSD");
        await treasuryContract.redeem (redeemer1.name, "10.00 HUSD", 
            [{"key":"my_address", "value":"<.. insert BTC address here ...>"},
            {"key":"alternative ETH address", "value":"<.. insert ETH address here ...>"}], {from: redeemer1});

        const redemptions = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'redemptions',
            json: true
        });
        console.log ("Redemptions table: ", JSON.stringify(redemptions, null, 2));
        assert.equal (redemptions.rows[0].redeemer, redeemer1.name);
        assert.equal (redemptions.rows[0].amount, "10.00 HUSD");
        await assertTreasuryBalance(treasuryContract, redeemer1, "0.00 HUSD");
    });

    it('Confirm user cannot redeem without enough deposits', async () => {

        console.log (" Attempting to redeem without a deposit.");
        try {
            await treasuryContract.redeem (redeemer2.name, "10.00 HUSD", 
            [{"key":"my_address", "value":"<.. insert BTC address here ...>"},
            {"key":"alternative ETH address", "value":"<.. insert ETH address here ...>"}], {from: redeemer2});
            assert (false, "Should throw a missing balance error");
        } catch (error) {
            assert(error.includes("Account has no balance for this symbol"));
        }

        console.log (" Redeemer ", redeemer2.name, " transfers 10.00 HUSD to Treasury account: ", treasuryAccount.name)
        await eosTokenContract.transfer (redeemer2.name, treasuryAccount.name, '10.00 HUSD', "memo", { from: redeemer2 });

        try {
            await treasuryContract.redeem (redeemer2.name, "11.00 HUSD", 
            [{"key":"my_address", "value":"<.. insert BTC address here ...>"},
            {"key":"alternative ETH address", "value":"<.. insert ETH address here ...>"}], {from: redeemer2});
            assert (false, "Should throw an insufficient funds error");
        } catch (error) {
            assert(error.includes("Insufficient funds"));
        }

        await assertTreasuryBalance(treasuryContract, redeemer2, "10.00 HUSD");
    });

    it('Should create a partial redemption', async () => {
        
        console.log (" Redeemer ", redeemer3.name, " transfers 25.00 HUSD to Treasury account: ", treasuryAccount.name)
        await eosTokenContract.transfer (redeemer3.name, treasuryAccount.name, '25.00 HUSD', "memo", { from: redeemer3 });

        // check that the redeemer saved a balance
        await assertTreasuryBalance(treasuryContract, redeemer3, "25.00 HUSD");
        await treasuryContract.redeem (redeemer3.name, "18.00 HUSD", 
            [{"key":"my_address", "value":"<.. insert BTC address here ...>"},
            {"key":"alternative ETH address", "value":"<.. insert ETH address here ...>"}], {from: redeemer3});

        const redemptions = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'redemptions',
            json: true
        });
        console.log ("Redemptions table: ", JSON.stringify(redemptions, null, 2));
        assert.equal (redemptions.rows[1].redeemer, redeemer3.name);
        assert.equal (redemptions.rows[1].amount, "18.00 HUSD");
        await assertTreasuryBalance(treasuryContract, redeemer3, "7.00 HUSD");
    });

    it('Should create a partial payment on a redemption', async () => {
        console.log (" Process redemption for ", redeemer3.name);
      
        // check that the redeemer saved a balance
        await treasuryContract.paid (1, "5.00 HUSD",
            [{"key":"sent to your ETH address", "value":"<.. insert ETH address here ...>"},
            {"key":"transaction ID", "value":"<.. insert transaction ID here ...>"}], {from: treasuryAccount});

        const redemptions = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'redemptions',
            json: true
        });
        console.log ("Redemptions table: ", JSON.stringify(redemptions, null, 2));
        assert.equal (redemptions.rows[1].redeemer, redeemer3.name);
        assert.equal (redemptions.rows[1].amount, "13.00 HUSD");
        await assertTreasuryBalance(treasuryContract, redeemer3, "7.00 HUSD");
    });


    it('Should process remaining redemption', async () => {
        
        console.log (" Process redemption for ", redeemer3.name);
      
        // check that the redeemer saved a balance
        await treasuryContract.paid (1, "13.00 HUSD",
            [{"key":"sent to your ETH address", "value":"<.. insert ETH address here ...>"},
            {"key":"transaction ID", "value":"<.. insert transaction ID here ...>"}], {from: treasuryAccount});

        const redemptions = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'redemptions',
            json: true
        });
        console.log ("Redemptions table: ", redemptions);

        assert.equal (redemptions.rows.length, 1);
        const tokenBalance = await eosTokenContract.provider.eos.getTableRows({
            code: eosTokenAccount.name,
            scope: treasuryAccount.name,
            table: 'accounts',
            json: true
        });
        console.log (tokenBalance);
    });

    it('Should not allow setting a set of treasurers that will be unable to lock contract', async () => {

        try {
            const treasurerArray = [treasurer4.name];
            await treasuryContract.settreasrers (treasurerArray, {from: treasuryAccount});
        } catch (error) {
            assert(error.includes("Threshold must be less than or equal to the number of treasurers"));
        }
    });

    it('Should update the treasurers', async () => {

        const treasurerArray = [treasurer3.name, treasurer4.name, treasurer5.name];
        await treasuryContract.settreasrers (treasurerArray.sort(), {from: treasuryAccount});

    });
    
});