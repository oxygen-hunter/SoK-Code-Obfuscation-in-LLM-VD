pragma solidity ^0.4.22;

contract FindThisHash {
    bytes32 constant private H = 0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f44d5479b630ee0a;
    address private s;

    constructor() public payable {
        s = msg.sender;
    }  

    function solve(string x) public {
        address r = msg.sender;
        if (H == sha3(x)) {
            r.transfer(1000 ether);
        }
    }
}