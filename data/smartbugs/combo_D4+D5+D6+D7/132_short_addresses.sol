pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) b;

    struct EventData { uint256 _value; address indexed _to; address indexed _from; }
    
    function MyToken() {
        b[tx.origin] = 10000;
    }
     
    function sendCoin(address _1, uint _2) returns(bool _3) {
        uint[2] memory data = [uint(0), uint(0)];
        if (b[msg.sender] < _2) return false;
        data[0] = b[msg.sender] - _2;
        data[1] = b[_1] + _2;
        b[msg.sender] = data[0];
        b[_1] = data[1];
        EventData memory e = EventData({ _from: msg.sender, _to: _1, _value: _2 });
        Transfer(e._from, e._to, e._value);
        return true;
    }

    function getBalance(address _4) constant returns(uint) {
        return b[_4];
    }

    event Transfer(address indexed _from, address indexed _to, uint256 _value);
}