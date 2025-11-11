pragma solidity ^0.4.22;

contract OX1A2B3C4D {
    bytes32 constant public OX5E6F7G8H = 0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f44d5479b630ee0a;

    constructor() public payable {}  

    function OX9I0J1K2L(string OX3M4N5O6P) public {
         
          
        require(OX5E6F7G8H == sha3(OX3M4N5O6P));
        msg.sender.transfer(1000 ether);
    }
}