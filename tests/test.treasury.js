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

async function getSupply (tokenContract) {
    const supply = await tokenContract.provider.eos.getTableRows({
        code: tokenContract.name,
        scope: 'HUSD',
        table: 'stat',
        json: true
    });
    if (supply.rows.length == 0) {
        return "0.00 HUSD";
    }
    return supply.rows[0].supply;
}

async function getPayments (treasury) {
    const payments = await treasury.provider.eos.getTableRows({
        code: treasury.name,
        scope: treasury.name,
        table: 'payments',
        json: true
    });
    return payments.rows;
}

async function getRedemptions (treasury) {
    const redemptions = await treasury.provider.eos.getTableRows({
        code: treasury.name,
        scope: treasury.name,
        table: 'redemptions',
        json: true
    });
    return redemptions.rows;
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
        const singletreas = await treasuryAccount.createSubAuthority ("singletreas", 1);
        await singletreas.setAuthorityAbilities([
            {
                action: 'newpayment',
                contract: treasuryAccount.name
            },
            {
                action: 'attestpaymnt',
                contract: treasuryAccount.name
            },
            {
                action: 'removeattest',
                contract: treasuryAccount.name
            }
        ]);

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
        console.log (" redeemer4            : ", redeemer4.name);
        console.log (" redeemer5            : ", redeemer5.name);

        const initialBalance = '1000.00 HUSD';

        treasuryContract = await eoslime.Contract.deployOnAccount (TREASURY_WASM_PATH, TREASURY_ABI_PATH, treasuryAccount);
        eosTokenContract = await eoslime.Contract.deployOnAccount (EOSIOTOKEN_WASM_PATH, EOSIOTOKEN_ABI_PATH, eosTokenAccount);
        console.log (" Treasury Contract   : ", treasuryContract.name);
        console.log (" HUSD Token Contract : ", eosTokenContract.name);

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
            [{"key":"paused","value":0},{"key":"redemption_paused","value":0}, {"key":"threshold","value":2}], { from: treasuryAccount});
        // await treasuryContract.togglepause ( {from: treasuryAccount} );
        await treasuryContract.setredsymbol("2,HUSD", { from: treasuryAccount });
        // await treasuryContract.settreasrers ({"treasurers":[treasurer1,treasurer2]}, {from: treasuryAccount});
        await treasuryContract.settreasrers ([treasurer1.name,treasurer2.name], [], {from: treasuryAccount});

        await eosTokenContract.create(treasuryAccount.name, '-1.00 HUSD', { from: eosTokenAccount });

        console.log ("\n\n");
        console.log (" Redeemers each receive ", initialBalance , " at the beginning of all tests.")
       
        await eosTokenContract.issue(treasuryAccount.name, '100000.00 HUSD', 'memo', { from: treasuryAccount});
        await eosTokenContract.transfer(treasuryAccount.name, redeemer1.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer2.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer3.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer4.name, initialBalance, 'memo', { from: treasuryAccount });
        await eosTokenContract.transfer(treasuryAccount.name, redeemer5.name, initialBalance, 'memo', { from: treasuryAccount });

        // console.log ("\nTreasury Configuration -----------------------")
        let configTable = await treasuryContract.provider.eos.getTableRows({
            code: treasuryContract.name,
            scope: treasuryContract.name,
            table: 'config',
            json: true
        });
        config = configTable.rows[0];
        // console.log (config, "\n\n");
    });

    beforeEach(async () => {
   
    });

    it('Should create a redemption', async () => {
        
        // console.log (" Redeemer ", redeemer1.name, " transfers 10.00 HUSD to Treasury account: ", treasuryAccount.name)
        await eosTokenContract.transfer (redeemer1.name, treasuryAccount.name, '10.00 HUSD', "memo", { from: redeemer1 });

        // check that the redeemer saved a balance
        await assertTreasuryBalance(treasuryContract, redeemer1, "10.00 HUSD");
        await treasuryContract.redeem (redeemer1.name, "10.00 HUSD", 
        [{"key":"my_address", "value":"b95a7b2d478600881281b3b69427775a"},
        {"key":"alternative ETH address", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"}], {from: redeemer1});

        const redemptions = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'redemptions',
            json: true
        });
        // console.log ("Redemptions table: ", JSON.stringify(redemptions, null, 2));
        assert.equal (redemptions.rows[0].requestor, redeemer1.name);
        assert.equal (redemptions.rows[0].amount_requested, "10.00 HUSD");
        await assertTreasuryBalance(treasuryContract, redeemer1, "0.00 HUSD");
    });

    it('Confirm user cannot redeem without enough deposits', async () => {

        // console.log (" Attempting to redeem without a deposit.");
        try {
            await treasuryContract.redeem (redeemer2.name, "10.00 HUSD", 
            [{"key":"my_address", "value":"b95a7b2d478600881281b3b69427775a"},
            {"key":"alternative ETH address", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"}], {from: redeemer2});
            assert (false, "Should throw a missing balance error");
        } catch (error) {
            assert(error.includes("Account has no balance for this symbol"));
        }

        // console.log (" Redeemer ", redeemer2.name, " transfers 10.00 HUSD to Treasury account: ", treasuryAccount.name)
        await eosTokenContract.transfer (redeemer2.name, treasuryAccount.name, '10.00 HUSD', "memo", { from: redeemer2 });

        try {
            await treasuryContract.redeem (redeemer2.name, "11.00 HUSD", 
            [{"key":"my_address", "value":"b95a7b2d478600881281b3b69427775a"},
            {"key":"alternative ETH address", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"}], {from: redeemer2});
            assert (false, "Should throw an insufficient funds error");
        } catch (error) {
            assert(error.includes("Insufficient funds"));
        }

        await assertTreasuryBalance(treasuryContract, redeemer2, "10.00 HUSD");
    });

    it('Should create a partial redemption', async () => {
        
        // console.log (" Redeemer ", redeemer3.name, " transfers 25.00 HUSD to Treasury account: ", treasuryAccount.name)
        await eosTokenContract.transfer (redeemer3.name, treasuryAccount.name, '25.00 HUSD', "memo", { from: redeemer3 });

        await assertTreasuryBalance(treasuryContract, redeemer3, "25.00 HUSD");
        await treasuryContract.redeem (redeemer3.name, "18.00 HUSD", 
        [{"key":"my_address", "value":"b95a7b2d478600881281b3b69427775a"},
        {"key":"alternative ETH address", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"}], {from: redeemer3});
    
        const redemptions = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'redemptions',
            json: true
        });
        // console.log ("Redemptions table: ", JSON.stringify(redemptions, null, 2));
        assert.equal (redemptions.rows[1].requestor, redeemer3.name);
        assert.equal (redemptions.rows[1].amount_requested, "18.00 HUSD");
        await assertTreasuryBalance(treasuryContract, redeemer3, "7.00 HUSD");
    });

    it('Should create a payment on a redemption', async () => {
        // console.log (" Process redemption for ", redeemer3.name);
      
        await treasuryContract.newpayment (treasurer1.name, 1, "5.00 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer1});

        const payments = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'payments',
            json: true
        });
        // console.log ("Payments table: ", JSON.stringify(payments, null, 2));
        assert.equal (payments.rows[0].creator, treasurer1.name);
        assert.equal (payments.rows[0].amount_paid, "5.00 HUSD");
    });

    it('Should attest to a payment', async () => {
              
        const payments1 = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'payments',
            json: true
        });

        // we are not burning at this time so supply should remain the same
        assert.equal (await getSupply(eosTokenContract), "100000.00 HUSD")

        try {
            await treasuryContract.attestpaymnt (treasurer1.name, 0, 1, "5.00 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer1});
        } catch (error) {
            assert(error.includes("already attested to this payment"));
        }

        await treasuryContract.attestpaymnt (treasurer2.name, 0, 1, "5.00 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer2});
        
        const payments = await treasuryContract.provider.eos.getTableRows({
            code: treasuryAccount.name,
            scope: treasuryAccount.name,
            table: 'payments',
            json: true
        });
        // console.log ("Payments table: ", JSON.stringify(payments, null, 2));

        // we are not burning at this time so supply should remain the same
        assert.equal (await getSupply(eosTokenContract), "100000.00 HUSD");

        assert.equal (payments.rows[0].creator, treasurer1.name);
        assert.equal (payments.rows[0].amount_paid, "5.00 HUSD");
        assert.equal (payments.rows[0].attestations.length, 2);
    });

    it('Should allow a new treasurer to create a new payment for the same redemption to finalize it', async () => {
        // console.log (" Process redemption for ", redeemer3.name);

        await treasuryContract.addtreasrer (treasurer3.name, [], {from: treasuryAccount});
        await treasuryContract.addtreasrer (treasurer4.name, [], {from: treasuryAccount});
      
        await treasuryContract.newpayment (treasurer3.name, 1, "13.00 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer3});

        await treasuryContract.attestpaymnt (treasurer4.name, 1, 1, "13.00 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer4});

        const payments = await getPayments (treasuryContract)
        const redemptions = await getRedemptions (treasuryContract)
        
        assert.equal (payments[1].creator, treasurer3.name);
        assert.equal (payments[1].amount_paid, "13.00 HUSD");

        // Redemption ID of 1 should be erased 
        // console.log ("Redemptions table: ", JSON.stringify(redemptions, null, 2));
        assert.equal (redemptions[1].amount_paid, "18.00 HUSD");
        assert.equal (redemptions[1].amount_requested, "18.00 HUSD");
        
        assert.equal (redemptions.length, 2);

        // we are not burning at this time so supply should remain the same
        assert.equal (await getSupply(eosTokenContract), "100000.00 HUSD");
    });

    it('Should allow a treasurer to create and then remove a payment/attestation', async () => {
        // console.log (" Process redemption for ", redeemer3.name);

        await eosTokenContract.transfer (redeemer4.name, treasuryAccount.name, '16.50 HUSD', "memo", { from: redeemer4 });

        // check that the redeemer saved a balance
        await assertTreasuryBalance(treasuryContract, redeemer4, "16.50 HUSD");
        await treasuryContract.redeem (redeemer4.name, "16.50 HUSD", 
            [{"key":"my_address", "value":"b95a7b2d478600881281b3b69427775a"},
            {"key":"alternative ETH address", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"}], {from: redeemer4});

        const redemptions = await getRedemptions (treasuryContract);
        assert.equal (redemptions[2].requestor, redeemer4.name);
        assert.equal (redemptions[2].amount_requested, "16.50 HUSD");
      
        await treasuryContract.newpayment (treasurer4.name, 2, "16.50 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer4});

        let payments = await getPayments (treasuryContract);   
        // console.log ("Before removing attestation: ", JSON.stringify(payments, null, 2));
        assert.equal (payments.length, 3);
        assert.equal (payments[2].creator, treasurer4.name);
        assert.equal (payments[2].amount_paid, "16.50 HUSD");
        
        await treasuryContract.removeattest (treasurer4.name, 2, 
            [{"key":"reason", "value":"accidently created the payment; it should be removed"}], {from: treasurer4});

        payments = await getPayments (treasuryContract);
        // console.log ("After removing attestation: ", JSON.stringify(payments, null, 2));
        assert.equal (payments.length, 2);
    });

    it('Should allow a 2nd treasurer add and then remove an attestation', async () => {
        // console.log (" Process redemption for ", redeemer3.name);
 
        await treasuryContract.newpayment (treasurer4.name, 2, "0.75 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer4});

        await treasuryContract.attestpaymnt (treasurer1.name, 2, 2, "0.75 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer1});

        let payments = await getPayments (treasuryContract);   
        // console.log ("Before removing attestation: ", JSON.stringify(payments, null, 2));
        assert.equal (payments.length, 3);
        assert.equal (payments[2].creator, treasurer4.name);
        assert.equal (payments[2].amount_paid, "0.75 HUSD");
        assert.equal (payments[2].attestations.length, 2);
        
        try {
            await treasuryContract.removeattest (treasurer4.name, 2, 
                [{"key":"reason", "value":"this is a different reason than the prior test"}], {from: treasurer4});
        } catch (error) {
            assert(error.includes("Cannot remove an attestation from a payment that has already been confirmed"));
        } 

        payments = await getPayments (treasuryContract);
        // console.log ("After removing attestation: ", JSON.stringify(payments, null, 2));
        assert.equal (payments.length, 3);
        assert.equal (payments[2].creator, treasurer4.name);
        assert.equal (payments[2].amount_paid, "0.75 HUSD");
        assert.equal (payments[2].attestations.length, 2);
    });

    it('Should allow a treasurer to add newpayment and then remove it by deleting their attestation', async () => {
 
        await treasuryContract.newpayment (treasurer4.name, 2, "0.55 HUSD",
            [{"key":"trx_id", "value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},
            {"key":"network", "value":"BTC"}], {from: treasurer4});

        let payments = await getPayments (treasuryContract);   
        assert.equal (payments.length, 4);
        assert.equal (payments[3].creator, treasurer4.name);
        assert.equal (payments[3].amount_paid, "0.55 HUSD");
        assert.equal (payments[3].attestations.length, 1);
        
        await treasuryContract.removeattest (treasurer4.name, 3, 
            [{"key":"reason", "value":"this is a different reason than the prior test"}], {from: treasurer4});

        payments = await getPayments (treasuryContract);
        assert.equal (payments.length, 3);
    });

    // it('Should not allow setting a set of treasurers that will be unable to lock contract', async () => {
    //     try {
    //         const treasurerArray = [treasurer4.name];
    //         await treasuryContract.settreasrers (treasurerArray, {from: treasuryAccount});
    //     } catch (error) {
    //         assert(error.includes("Threshold must be less than or equal to the number of treasurers"));
    //     }
    // });

    // it('Should update the treasurers', async () => {

    //     const treasurerArray = [treasurer3.name, treasurer4.name, treasurer5.name];
    //     await treasuryContract.settreasrers (treasurerArray.sort(), {from: treasuryAccount});

    // });
    
});