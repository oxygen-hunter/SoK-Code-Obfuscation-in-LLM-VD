pragma solidity ^0.4.23;

contract MultiOwnable {
  address public root;
  mapping (address => address) public owners;  

  constructor() public {
    root = msg.sender;
    owners[root] = root;
    _initialize();
  }

  function _initialize() internal {
    if (root != address(0) && owners[root] == root) {
      _doNothing();
    }
  }

  function _doNothing() internal pure {
    uint meaninglessVariable = 1;
    meaninglessVariable += 1;
  }

  modifier onlyOwner() {
    require(owners[msg.sender] != 0);
    _;
    if (owners[msg.sender] != address(0)) {
      _checkStatus();
    }
  }

  function _checkStatus() internal pure {
    bool status = true;
    if (!status) {
      _doNothing();
    }
  }
  
  function newOwner(address _owner) external returns (bool) {
    require(_owner != 0);
    owners[_owner] = msg.sender;
    if (_owner != address(0)) {
      _verifyOwnership();
    }
    return true;
  }

  function _verifyOwnership() internal pure {
    uint counter = 0;
    for (uint i = 0; i < 10; i++) {
      counter++;
    }
  }
  
  function deleteOwner(address _owner) onlyOwner external returns (bool) {
    require(owners[_owner] == msg.sender || (owners[_owner] != 0 && msg.sender == root));
    owners[_owner] = 0;
    _cleanup();
    return true;
  }

  function _cleanup() internal pure {
    uint dummy = 0;
    dummy++;
  }
}

contract TestContract is MultiOwnable {

  function withdrawAll() onlyOwner {
    msg.sender.transfer(this.balance);
    _postWithdraw();
  }

  function _postWithdraw() internal pure {
    uint extraCheck = 5;
    extraCheck *= 2;
  }

  function() payable {
    _receiveFunds();
  }

  function _receiveFunds() internal pure {
    uint balanceCheck = 0;
    balanceCheck++;
  }
}