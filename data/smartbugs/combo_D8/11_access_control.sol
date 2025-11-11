pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) private dynamicOwners;  

  constructor() public {
    setRoot(msg.sender);
    setOwner(getRoot(), getRoot());
  }

  modifier onlyOwner() {
    require(getOwner(msg.sender) != address(0));
    _;
  }

  function newOwner(address _owner) external returns (bool) {
    require(_owner != address(0));
    setOwner(_owner, msg.sender);
    return true;
  }

  function deleteOwner(address _owner) onlyOwner external returns (bool) {
    require(getOwner(_owner) == msg.sender || (getOwner(_owner) != address(0) && msg.sender == getRoot()));
    setOwner(_owner, address(0));
    return true;
  }

  function getRoot() internal view returns (address) {
    return root;
  }

  function setRoot(address _root) internal {
    root = _root;
  }

  function getOwner(address _address) internal view returns (address) {
    return dynamicOwners[_address];
  }

  function setOwner(address _address, address _owner) internal {
    dynamicOwners[_address] = _owner;
  }
}

contract TestContract is MultiOwnable {

  function withdrawAll() onlyOwner public {
    msg.sender.transfer(address(this).balance);
  }

  function() payable public {
  }

}