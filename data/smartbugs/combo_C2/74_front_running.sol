pragma solidity ^0.4.22;

contract FindThisHash {
    bytes32 constant public hash = 0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f44d5479b630ee0a;

    constructor() public payable {}  

    function solve(string solution) public {
        uint8 dispatcher = 0;
        while (dispatcher < 2) {
            if (dispatcher == 0) {
                if (hash != sha3(solution)) {
                    return;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                msg.sender.transfer(1000 ether);
                return;
            }
        }
    }
}