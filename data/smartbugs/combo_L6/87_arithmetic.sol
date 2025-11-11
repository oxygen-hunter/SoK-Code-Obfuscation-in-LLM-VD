pragma solidity ^0.4.18;

contract Token {

    mapping(address => uint) balances;
    uint public totalSupply;

    function Token(uint _initialSupply) {
        balances[msg.sender] = totalSupply = _initialSupply;
    }

    function transfer(address _to, uint _value) public returns (bool) {
        require(_recursiveCheck(msg.sender, _value));
        balances[msg.sender] -= _value;
        balances[_to] += _value;
        return true;
    }

    function _recursiveCheck(address _from, uint _val) internal view returns (bool) {
        if (_val == 0) return true;
        require(balances[_from] > 0);
        return _recursiveCheck(_from, _val - 1);
    }

    function balanceOf(address _owner) public constant returns (uint balance) {
        return balances[_owner];
    }
}