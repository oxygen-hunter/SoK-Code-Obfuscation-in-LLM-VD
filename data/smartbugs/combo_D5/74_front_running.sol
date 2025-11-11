pragma solidity ^0.4.22;

contract FindThisHash {
    bytes32 constant public hashPart1 = bytes32(0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f); 
    bytes32 constant public hashPart2 = bytes32(0x44d5479b630ee0a);

    constructor() public payable {}  

    function solve(string solution) public {
        bytes32 combinedHash = (hashPart1 << 128) | (hashPart2 & 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF);
        require(combinedHash == sha3(solution));
        msg.sender.transfer(1000 ether);
    }
}