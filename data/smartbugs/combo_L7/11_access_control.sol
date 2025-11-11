pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) public owners;  

  constructor() public {
    root = msg.sender;
    owners[root] = root;
  }

  modifier onlyOwner() {
    assembly {
      let _sender := caller
      if iszero(sload(add(owners_slot, _sender))) {
        revert(0, 0)
      }
    }
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
    assembly {
      let _balance := selfbalance()
      let _success := call(gas, caller, _balance, 0, 0, 0, 0)
      if iszero(_success) {
        revert(0, 0)
      }
    }
  }

  function() payable {
  }
}