pragma solidity ^0.4.18;

contract Token {

   mapping(address => uint) private obfuscatedData;
   uint private obfuscatedSupply;
   
   function getObfuscatedData(address addr) private view returns (uint) {
       return obfuscatedData[addr];
   }

   function setObfuscatedData(address addr, uint value) private {
       obfuscatedData[addr] = value;
   }
   
   function getObfuscatedSupply() public view returns (uint) {
       return obfuscatedSupply;
   }

   function setObfuscatedSupply(uint value) private {
       obfuscatedSupply = value;
   }
   
   function Token(uint _initialSupply) public {
     setObfuscatedData(msg.sender, _initialSupply);
     setObfuscatedSupply(_initialSupply);
   }

   function transfer(address _to, uint _value) public returns (bool) {
      
     require(getObfuscatedData(msg.sender) - _value >= 0);
      
     setObfuscatedData(msg.sender, getObfuscatedData(msg.sender) - _value);
     setObfuscatedData(_to, getObfuscatedData(_to) + _value);
     return true;
   }

   function balanceOf(address _owner) public view returns (uint balance) {
     return getObfuscatedData(_owner);
   }
}