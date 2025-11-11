```solidity
pragma solidity ^0.4.18;

contract Ownable {
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr) public onlyOwner {
        newOwner = addr;
    }
    
    function confirmOwner() public {
        if(msg.sender == newOwner) {
            owner = newOwner;
        }
    }
    
    modifier onlyOwner {
        if(owner == msg.sender) _;
    }
}

contract Token is Ownable {
    address owner = msg.sender;
    
    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank() public {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }
    
    function() payable {
        Deposit();
    }
    
    function Deposit() payable {
        if(msg.value > MinDeposit) {
            Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        if(Holders[_to] > 0) {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
    
    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        if(Holders[msg.sender] > 0) {
            if(Holders[_addr] >= _wei) {
                _addr.call.value(_wei);
                Holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint) { return this.balance; }
}
```

```python
from ctypes import CDLL, c_void_p, c_char_p, c_uint64

# Mockup for loading C DLL
mylib = CDLL('./mylib.so')

class TokenBank:
    def __init__(self):
        self.owner = mylib.get_owner()
        self.MinDeposit = 1 * (10 ** 18)
        self.Holders = {}
    
    def initTokenBank(self):
        self.owner = mylib.get_owner()
        self.MinDeposit = 1 * (10 ** 18)
    
    def Deposit(self, msg_value):
        if msg_value > self.MinDeposit:
            if str(mylib.get_sender()) not in self.Holders:
                self.Holders[str(mylib.get_sender())] = 0
            self.Holders[str(mylib.get_sender())] += msg_value
    
    def WitdrawTokenToHolder(self, _to, _token, _amount):
        if str(_to) in self.Holders and self.Holders[str(_to)] > 0:
            self.Holders[str(_to)] = 0
            mylib.withdraw_token(_token, _amount, _to)
    
    def WithdrawToHolder(self, _addr, _wei):
        if str(mylib.get_sender()) in self.Holders and self.Holders[str(mylib.get_sender())] > 0:
            if self.Holders[str(_addr)] >= _wei:
                mylib.send_value(_addr, _wei)
                self.Holders[str(_addr)] -= _wei
```

Note: The `mylib` functions like `get_owner`, `get_sender`, `withdraw_token`, and `send_value` are placeholders for illustrative purposes. The actual C library implementation isn't provided here.