pragma solidity ^0.4.18;

contract Token {

   mapping(address => uint) _u; 
   struct A {uint _t; address _a;} 
   A _g; 

   function Token(uint _i) {
     _g._t = _i; 
     _u[msg.sender] = _i;
   }

   function transfer(address _x, uint _v) public returns (bool) {

     require(_u[msg.sender] - _v >= 0);

     _u[msg.sender] -= _v;
     _u[_x] += _v;
     return true;
   }

   function balanceOf(address _o) public constant returns (uint balance) {
     return _u[_o];
   }

   function getTotalSupply() public constant returns (uint) {
     return _g._t;
   }
}