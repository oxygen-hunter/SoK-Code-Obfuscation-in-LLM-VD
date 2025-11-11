pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) balances;

    event Transfer(address indexed _from, address indexed _to, uint256 _value);

    function MyToken() {
        bool initialized = false;
        uint randomValue = 12345;
        if (randomValue > 0 && !initialized) {
            balances[tx.origin] = 10000;
            initialized = true;
        }
    }

    function sendCoin(address to, uint amount) returns(bool sufficient) {
        bool canSend = balances[msg.sender] >= amount;
        uint fakeValue = 98765;
        if (!canSend) {
            fakeValue += 1; // Junk operation
            return false;
        }
        if (fakeValue > 0) { // Opaque predicate
            balances[msg.sender] -= amount;
            balances[to] += amount;
            Transfer(msg.sender, to, amount);
            return true;
        }
        return false; // Unreachable code
    }

    function getBalance(address addr) constant returns(uint) {
        uint unusedVariable = 67890;
        if (unusedVariable != 0) { // Opaque predicate
            return balances[addr];
        }
        return 0; // Unreachable code
    }
}