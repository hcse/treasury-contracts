cleos -u https://api.telos.kitchen push action -sjd -x 86400 bank.hypha paid '{"redemption_id":7, "amount":"6224.00 HUSD", "notes":[{"key":"trx_id","value":"77af928d57e7666436e31c8c0b5e73f9b95a7b2d478600881281b3b69427775a"},{"key":"network","value":"BTC"}]}' -p hyphanewyork > mark_paid.json

cleos -u https://api.telos.kitchen multisig propose_trx markpaid '[{"actor": "hyphanewyork", "permission": "active"}, {"actor": "cometogether", "permission": "active"}, {"actor":"illumination", "permission":"active"}, {"actor":"thealchemist", "permission":"active"}, {"actor":"joachimstroh", "permission":"active"}]' ./mark_paid.json hyphanewyork 

cleos -u https://api.telos.kitchen multisig approve hyphanewyork markpaid '{"actor": "hyphanewyork", "permission": "active"}' -p hyphanewyork

cleos -u https://api.telos.kitchen multisig exec hyphanewyork markpaid -p hyphanewyork