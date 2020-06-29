# transfer and redemption requests
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json transfer johnnyhypha1 bank.hypha "1.00 HUSD" --contract husd.hypha
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json tx create bank.hypha redeem '{"requestor":"johnnyhypha1", "amount":"1.00 HUSD", "notes":[{"key":"note #1","value":"please redeem"}]}' -p johnnyhypha1

# payment added by the treasurer that handled it
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json tx create bank.hypha newpayment '{"treasurer":"treasureriii", "amount":"1.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p treasureriii

# redemption should be removed
eosc -u https://test.telos.kitchen get table bank.hypha bank.hypha redemptions















# set contract
eosc -u https://test.telos.kitchen --vault-file ../eosc-testnet-vault.json system setcontract bank.hypha treasury/treasury.wasm treasury/treasury.abi






----------------------


cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":0, "amount":"10.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":1, "amount":"21.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":2, "amount":"3.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":3, "amount":"21.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
cleos -u https://test.telos.kitchen push action bank.hypha paid '{"redemption_id":4, "amount":"1.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p bank.hypha
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