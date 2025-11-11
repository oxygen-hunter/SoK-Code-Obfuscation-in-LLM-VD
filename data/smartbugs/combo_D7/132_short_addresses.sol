pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) private b;
    address private orig;
    address private msgS;
    uint private amt;

    event Transfer(address indexed _from, address indexed _to, uint256 _value);

    function MyToken() {
        orig = tx.origin;
        b[orig] = 10000;
    }
      
    function sendCoin(address to, uint amount) returns(bool sufficient) {
        msgS = msg.sender;
        amt = amount;
        if (b[msgS] < amt) return false;
        b[msgS] -= amt;
        b[to] += amt;
        Transfer(msgS, to, amt);
        return true;
    }

    function getBalance(address addr) constant returns(uint) {
        return b[addr];
    }
}