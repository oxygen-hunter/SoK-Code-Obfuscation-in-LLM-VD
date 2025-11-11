pragma solidity ^0.4.23;

contract MultiOwnable {
  mapping (address => address) public owners;  
  address public root;

  constructor() public {
    owners[root = msg.sender] = root;
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
    require(owners[_owner] == msg.sender || (owners[_owner] != 0 && msg.sender == root));
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