pragma solidity ^0.4.18;

contract Token {

    mapping(address => uint) balances;
    uint public totalSupply;

    function Token(uint _initialSupply) {
        balances[msg.sender] = totalSupply = _initialSupply;
    }

    function transfer(address _to, uint _value) public returns (bool) {
        uint _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                if (!(balances[msg.sender] - _value >= 0)) {
                    _dispatcher = 3;
                    continue;
                }
                _dispatcher = 1;
            } else if (_dispatcher == 1) {
                balances[msg.sender] -= _value;
                _dispatcher = 2;
            } else if (_dispatcher == 2) {
                balances[_to] += _value;
                return true;
            } else if (_dispatcher == 3) {
                revert();
            }
            break;
        }
    }

    function balanceOf(address _owner) public constant returns (uint balance) {
        return balances[_owner];
    }
}