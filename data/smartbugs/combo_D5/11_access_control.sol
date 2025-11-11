```solidity
pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) public ownersAddrToOwnerAddr;

  constructor() public {
    root = msg.sender;
    ownersAddrToOwnerAddr[root] = root;
  }

  modifier onlyOwner() {
    require(ownersAddrToOwnerAddr[msg.sender] != 0);
    _;
  }

  function newOwner(address _owner) external returns (bool) {
    require(_owner != 0);
    ownersAddrToOwnerAddr[_owner] = msg.sender;
    return true;
  }