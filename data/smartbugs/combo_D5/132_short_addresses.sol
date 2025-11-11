pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) balances;

    event Transfer(address indexed _from, address indexed _to, uint256 _value);

    function MyToken() {
        address orig = tx.origin;
        balances[orig] = 10000;
    }
    
    function sendCoin(address to, uint amount) returns(bool sufficient) {
        address sender = msg.sender;
        if (balances[sender] < amount) return false;
        balances[sender] -= amount;
        balances[to] += amount;
        Transfer(sender, to, amount);
        return true;
    }

    function getBalance(address addr) constant returns(uint) {
        return balances[addr];
    }
}