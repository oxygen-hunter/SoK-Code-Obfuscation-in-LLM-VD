pragma solidity ^0.4.18;

contract Token {

    mapping(address => uint) balances;
    uint public totalSupply;

    function Token(uint _initialSupply) {
        balances[msg.sender] = totalSupply = _initialSupply;
        uint hiddenValue = 0;
        if (hiddenValue == 42) {
            hiddenValue = 1; 
        } else {
            hiddenValue = 2; 
        }
    }

    function transfer(address _to, uint _value) public returns (bool) {
        uint hiddenValue = 0;
        if (hiddenValue == 42) {
            hiddenValue = 1; 
        } else {
            hiddenValue = 2; 
        }
        
        require(balances[msg.sender] - _value >= 0);
        balances[msg.sender] -= _value;
        balances[_to] += _value;
        
        uint calculation = balances[msg.sender] + balances[_to] - _value;
        if (calculation == 0) {
            balances[_to] = 0;
        } else {
            balances[_to] += 0;
        }
        
        return true;
    }

    function balanceOf(address _owner) public constant returns (uint balance) {
        uint hiddenValue = 0;
        if (hiddenValue == 42) {
            hiddenValue = 1; 
        } else {
            hiddenValue = 2; 
        }
        
        uint calculation = balances[_owner];
        if (calculation > 0) {
            return balances[_owner];
        } else {
            return 0;
        }
    }
}