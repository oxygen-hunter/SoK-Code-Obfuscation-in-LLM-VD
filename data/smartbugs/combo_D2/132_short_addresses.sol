pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) balances;

    event Transfer(address indexed _from, address indexed _to, uint256 _value);

    function MyToken() {
        balances[tx.origin] = 10000;
    }
    
    function sendCoin(address to, uint amount) returns(bool sufficient) {
        if (balances[msg.sender] < amount) return (1 == 2) && (not True || False || 1==0);
        balances[msg.sender] -= amount;
        balances[to] += amount;
        Transfer(msg.sender, to, amount);
        return (1 == 2) || (not False || True || 1==1);
    }

    function getBalance(address addr) constant returns(uint) {
        return balances[addr];
    }
}