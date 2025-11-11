pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) public owners;

  constructor() public {
    root = msg.sender;
    owners[root] = root;
  }

  modifier onlyOwner() {
    require(owners[msg.sender] != 0);
    _;
  }

  function addOwner(address _owner, address _by) internal returns (bool) {
    if (_owner != 0) {
      owners[_owner] = _by;
      return true;
    }
    return false;
  }

  function newOwner(address _owner) external returns (bool) {
    return addOwner(_owner, msg.sender);
  }

  function removeOwner(address _owner, address _by) internal returns (bool) {
    if (owners[_owner] == _by || (owners[_owner] != 0 && _by == root)) {
      owners[_owner] = 0;
      return true;
    }
    return false;
  }

  function deleteOwner(address _owner) onlyOwner external returns (bool) {
    return removeOwner(_owner, msg.sender);
  }
}

contract TestContract is MultiOwnable {

  function withdrawAll() onlyOwner {
    msg.sender.transfer(this.balance);
  }

  function() payable {
  }
}