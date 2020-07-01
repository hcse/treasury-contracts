# transfer and redemption requests
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json transfer johnnyhypha1 bank.hypha "1.00 HUSD" --contract husd.hypha
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json tx create bank.hypha redeem '{"requestor":"johnnyhypha1", "amount":"1.00 HUSD", "notes":[{"key":"note #1","value":"please redeem"}]}' -p johnnyhypha1

# payment added by the treasurer that handled it
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json tx create bank.hypha newpayment '{"treasurer":"treasureriii", "redemption_id":0, "amount":"1.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p treasureriii

# attest that a payment was made (as a witness treasurer)
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json tx create bank.hypha attestpaymnt '{"treasurer":"treasurermmm", "redemption_id":0, "payment_id":0, "amount":"1.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p treasurermmm

# remove an attestation 
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json tx create bank.hypha removeattest '{"treasurer":"treasureriii", "payment_id":0, "notes":[{"key":"reason","value":"I made a mistake and need to remove my attestation"}]}' -p treasureriii

# redemption should be removed
eosc -u https://test.telos.kitchen get table bank.hypha bank.hypha redemptions
eosc -u https://test.telos.kitchen get table bank.hypha bank.hypha payments




# reset 
eosc -u https://api.telos.kitchen --vault-file ../.json tx create bank.hypha unpauseredmp '{"":""}' -p bank.hypha

# set contract
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json system setcontract bank.hypha treasury/treasury.wasm treasury/treasury.abi



----------------------


eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":1, "amount":"1000.00 HUSD", "notes":[{"key":"trx_id","value":"0x41a8b9dcf4d217309f7adda80060ac6a324e9c4f3275d8a05eb4255becab7c22"},{"key":"network","value":"ETH-USDT"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":3, "amount":"3584.00 HUSD", "notes":[{"key":"trx_id","value":"6e623ea774e57b5301811d8a190f27588c50179ed1a7fbdacb59bf6f3c0392ef"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":4, "amount":"1697.00 HUSD", "notes":[{"key":"trx_id","value":"0x41a8b9dcf4d217309f7adda80060ac6a324e9c4f3275d8a05eb4255becab7c22"},{"key":"network","value":"ETH-USDT"}]}' -p bank.hypha

eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":7, "amount":"6224.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":8, "amount":"425.00 HUSD", "notes":[{"key":"trx_id","value":"ad68f172fd75f9758a3b4b08eace3777b060964a816536acb9f900b21f090218"},{"key":"network","value":"BTC"}]}' -p bank.hypha

eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":9, "amount":"851.00 HUSD", "notes":[{"key":"trx_id","value":"c83c44742e512df28caf190a379386268ae4d942a800e45bf1a36a95dcc7ea15"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":13, "amount":"851.00 HUSD", "notes":[{"key":"trx_id","value":"c83c44742e512df28caf190a379386268ae4d942a800e45bf1a36a95dcc7ea15"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":16, "amount":"850.00 HUSD", "notes":[{"key":"trx_id","value":"c83c44742e512df28caf190a379386268ae4d942a800e45bf1a36a95dcc7ea15"},{"key":"network","value":"BTC"}]}' -p bank.hypha

eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":10, "amount":"2562.00 HUSD", "notes":[{"key":"trx_id","value":"b5cbe25177abc05bb32fa4f136030452fed93e6b8658174c740266ef6a73b54e"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":11, "amount":"3500.00 HUSD", "notes":[{"key":"trx_id","value":"5dc5da02d423c0f5d738ea221b6328b99f224d537d666e0a4e37d64145c22e36"},{"key":"network","value":"BTC"}]}' -p bank.hypha

eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":14, "amount":"2500.00 HUSD", "notes":[{"key":"trx_id","value":"5dc5da02d423c0f5d738ea221b6328b99f224d537d666e0a4e37d64145c22e36"},{"key":"network","value":"BTC"}]}' -p bank.hypha

eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":12, "amount":"20.00 HUSD", "notes":[{"key":"trx_id","value":"487d7b1a3a29ee45df031adb4a942c7fa64e6db4211e17ed570c7e2044740851"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":19, "amount":"2000.00 HUSD", "notes":[{"key":"trx_id","value":"487d7b1a3a29ee45df031adb4a942c7fa64e6db4211e17ed570c7e2044740851"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":20, "amount":"4000.00 HUSD", "notes":[{"key":"trx_id","value":"487d7b1a3a29ee45df031adb4a942c7fa64e6db4211e17ed570c7e2044740851"},{"key":"network","value":"BTC"}]}' -p bank.hypha

eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":15, "amount":"273.00 HUSD", "notes":[{"key":"trx_id","value":"ef6b12a6c14a95b68a2dba2ea44b1b4bd9022d477dda64feda2c9d521b84daed"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":17, "amount":"5000.00 HUSD", "notes":[{"key":"trx_id","value":"e10b568180ac77957f68683e9f7b10cc4bceda019b831931766fe7b82cb61122"},{"key":"network","value":"BTC"}]}' -p bank.hypha
eosc -u https://api.telos.kitchen --vault-file ../daoctl/hyphanewyork.json tx create bank.hypha paid '{"redemption_id":18, "amount":"4018.00 HUSD", "notes":[{"key":"trx_id","value":"3a1410e9cb8037c1d2e506598c640dfdd0564d5cf0a6a51c75e5196e0755529f"},{"key":"network","value":"BTC"},{"key":"trx_id","value":"a6673afbb30e5935c0b0013f5b67d9cf8b0790fa8ba11e249fe77342f1d39e78"},{"key":"network","value":"BTC"}]}' -p bank.hypha



cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":4, "amount":"3.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":7, "amount":"21.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":8, "amount":"1.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha


cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":5, "amount":"2.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":6, "amount":"3.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":7, "amount":"4.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha


cleos -u https://test.telos.kitchen push action eosio updateauth '{
    "account": "hypha",
    "permission": "active",
    "parent": "owner",
    "auth": {
        "keys": [
            {
                "key": "EOS7PJ3hWdozwGZPdTFvuZhAvPakuKTGy4SJfweLGB7Pgu9sU9aW7",
                "weight": 1
            }
        ],
        "threshold": 1,
        "accounts": [
            {
                "permission": {
                    "actor": "hypha",
                    "permission": "eosio.code"
                },
                "weight": 1
            }
        ],
        "waits": []
    }
}' -p hypha@owner

cleos -u https://api.telos.kitchen push action eosio updateauth '{
    "account": "alice.tk",
    "permission": "owner",
    "parent": "",
    "auth": {
        "keys": [
            {
                "key": "EOS8Y2bVJDB6f1GWLBuyQA73wPSTj4DriHwc1nchS3brQc277BURk",
                "weight": 1
            }
        ],
        "threshold": 1,
        "accounts": [],
        "waits": []
    }
}' -p alice.tk@owner

eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json tx create bank.hypha setconfig '{
    "redemption_symbol": "2,HUSD",
    "names": [{
        "key": "dao_contract",
        "value": "dao.hypha"
      },{
        "key": "token_redemption_contract",
        "value": "husd.hypha"
      }
    ],
    "strings": [],
    "assets": [],
    "ints": [{
        "key": "paused",
        "value": 0
      },{
        "key": "redemption_paused",
        "value": 0
      },{
        "key": "threshold",
        "value": 1
      }
    ]}' -p bank.hypha