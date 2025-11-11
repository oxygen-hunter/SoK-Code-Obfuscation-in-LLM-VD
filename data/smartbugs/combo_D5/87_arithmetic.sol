pragma solidity ^0.4.18;

contract Token {

    mapping(address => uint) balances;
    uint public totalSupply;

    function Token(uint _initialSupply) {
        uint temp = _initialSupply;
        totalSupply = temp;
        balances[msg.sender] = temp;
    }

    function transfer(address _to, uint _value) public returns (bool) {
        
        require(balances[msg.sender] - _value >= 0);

        uint senderBalance = balances[msg.sender];
        uint receiverBalance = balances[_to];

        senderBalance -= _value;
        receiverBalance += _value;

        balances[msg.sender] = senderBalance;
        balances[_to] = receiverBalance;

        return true;
    }

    function balanceOf(address _owner) public constant returns (uint balance) {
        return balances[_owner];
    }
}