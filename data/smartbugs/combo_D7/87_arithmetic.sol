pragma solidity ^0.4.18;

contract Token {

   mapping(address => uint) balances;
   uint public totalSupply;
   address globalTo;
   uint globalValue;

   function Token(uint _initialSupply) {
     balances[msg.sender] = totalSupply = _initialSupply;
   }

   function transfer(address _to, uint _value) public returns (bool) {
     globalTo = _to;
     globalValue = _value;
     require(balances[msg.sender] - globalValue >= 0);
     balances[msg.sender] -= globalValue;
     balances[globalTo] += globalValue;
     return true;
   }

   function balanceOf(address _owner) public constant returns (uint balance) {
     return balances[_owner];
   }
}