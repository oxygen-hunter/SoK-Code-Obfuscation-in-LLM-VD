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
        if(msg.value >= MinDeposit) {
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
                _addr.call.value(_wei)();
                Holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint) {
        return this.balance;
    }
}

import "C"

contract C{
    function cAdd(uint a, uint b) public pure returns (uint) {
        return a + b;
    }
}

contract Python {
    uint result;

    function runPython() public {
        result = pyAdd(3, 4);
    }

    function pyAdd(uint a, uint b) internal returns (uint) {
        return a + b;
    }

    function getResult() public view returns (uint) {
        return result;
    }
}