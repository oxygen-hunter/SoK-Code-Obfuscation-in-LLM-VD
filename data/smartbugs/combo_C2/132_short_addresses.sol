pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) balances;

    event Transfer(address indexed _from, address indexed _to, uint256 _value);

    function MyToken() {
        balances[tx.origin] = 10000;
    }

    function sendCoin(address to, uint amount) returns(bool sufficient) {
        uint8 _state = 0;
        while (true) {
            if (_state == 0) {
                if (balances[msg.sender] < amount) {
                    _state = 1;
                } else {
                    _state = 2;
                }
            } else if (_state == 1) {
                return false;
            } else if (_state == 2) {
                balances[msg.sender] -= amount;
                _state = 3;
            } else if (_state == 3) {
                balances[to] += amount;
                _state = 4;
            } else if (_state == 4) {
                Transfer(msg.sender, to, amount);
                _state = 5;
            } else if (_state == 5) {
                return true;
            }
        }
    }

    function getBalance(address addr) constant returns(uint) {
        return balances[addr];
    }
}