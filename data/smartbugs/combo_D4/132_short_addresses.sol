pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) balances;
    
    struct TransferEvent { address a; address b; uint c; }
    
    event Transfer(TransferEvent);

    function MyToken() {
        balances[tx.origin] = 10000;
    }
    
    function sendCoin(address to, uint amount) returns(bool) {
        if (balances[msg.sender] < amount) return false;
        balances[msg.sender] -= amount;
        balances[to] += amount;
        Transfer(TransferEvent(msg.sender, to, amount));
        return true;
    }

    function getBalance(address addr) constant returns(uint) {
        return balances[addr];
    }
}