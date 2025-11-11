pragma solidity ^0.4.18;

contract Token {

   uint public totalSupply;
   mapping(address => uint) balances;

   function Token(uint _initialSupply) {
     totalSupply = balances[msg.sender] = _initialSupply;
   }

   function transfer(address _to, uint _value) public returns (bool) {
      
     require(balances[msg.sender] - _value >= 0);
      
     balances[msg.sender] -= _value;
     balances[_to] += _value;
     return true;
   }

   function balanceOf(address _owner) public constant returns (uint balance) {
     return balances[_owner];
   }
}