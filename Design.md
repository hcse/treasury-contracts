# Detailed Treasury Design/Flow

## Step 1: DHO Approves and Issues HUSD
Members vote on proposals to approve issuing/minting HUSD tokens.  
![image](https://user-images.githubusercontent.com/32852271/92121930-db7f0c00-edc8-11ea-8d45-c7286bd78298.png)

## Step 2: Member Requests Redemption of HUSD
At anytime, member sends HUSD tokens back to treasury for redemption.
![image](https://user-images.githubusercontent.com/32852271/92122253-37499500-edc9-11ea-9dc4-dfcb3656a7f9.png)

## Step 3: Multisig Transaction to Send BTC
Every two weeks, treasurers kick off a 'batch' of redemption payments.  The Copay BTC multisig wallet is used to send BTC, and it requires 3 of 5 treasurers to approve each transaction.
![image](https://user-images.githubusercontent.com/32852271/92126556-5139a680-edce-11ea-9b49-8c37debcd68a.png)

## Step 4: Record Payment in Treasury
A treasurer enters the BTC payment details into the DHO HUSD treasury.
![image](https://user-images.githubusercontent.com/32852271/92129497-99a69380-edd1-11ea-94c3-28111b40b4e8.png)

## Step 5: Record Attestations from Other Treasurers
This is the step that we are not achieving, and we must decide if it is important for us. Two additional treasurers (to meet 3 of 5 threshold) must attest that the payment submitted to the treasury is valid.  When this occurs:
1. Redemption request is marked as closed/confirmed.
1. HUSD is burned.
![image](https://user-images.githubusercontent.com/32852271/92127823-b04beb00-edcf-11ea-8b53-a0e600fbb90e.png)

This is an important step if we wish to maintain accurate visibility to HUSD total supply (liability). 
### Questions
1. Should the redemption request be closed with only one treasurer creating the payment on chain?
1. If not, perhaps we force the behavior by disallowing new redemption requests until old requests are cleared.  This would force the treasurers to comply or members would vote them out (b/c their redemptions would be blocked). 

# Income Statement / Balance Sheet Process
Separate and apart from the above process, the transactions are entered into a legacy-style ledger file in order to produce an income statement and balance sheet.  This process is very time consuming.  Each transaction is manually entered in the following format:

```
7/28	Luigi $2100 - $100 = $2000
		Assets:Hypha Treasury:BTC:Vault #3				-$2,100.00			; 42506952b25a7377c315fb7374c2c9a1bc22eb8cb1e00767d88085ff50020d76
		Expenses:Passport:luigicarlini
```

This process uses [hledger](https://hledger.org/), which is open source, plaintext double-entry accounting software similar to Quickbooks.

### Questions
1. Are views of the income statement and balance sheet important? I.e. should this work continue to be funded?
