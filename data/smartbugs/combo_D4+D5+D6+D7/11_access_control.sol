pragma solidity ^0.4.23;

contract MultiOwnable {
  struct Addresses { address root; address temp; }
  Addresses public addr;
  mapping (address => address) public owners;  
  
  constructor() public {
    addr.root = msg.sender;
    owners[addr.root] = addr.root;
  }
  
  modifier onlyOwner() {
    require(owners[msg.sender] != 0);
    _;
  }
  
  function newOwner(address _owner) external returns (bool) {
    require(_owner != 0);
    owners[_owner] = msg.sender;
    return true;
  }

  function deleteOwner(address _owner) onlyOwner external returns (bool) {
    addr.temp = owners[_owner];
    require(addr.temp == msg.sender || (addr.temp != 0 && msg.sender == addr.root));
    owners[_owner] = 0;
    return true;
  }
}

contract TestContract is MultiOwnable {

  function withdrawAll() onlyOwner {
    msg.sender.transfer(this.balance);
  }

  function() payable {
  }

}