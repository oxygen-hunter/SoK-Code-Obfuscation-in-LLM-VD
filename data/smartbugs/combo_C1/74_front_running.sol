pragma solidity ^0.4.22;

contract FindThisHash {
    bytes32 constant public hash = 0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f44d5479b630ee0a;

    constructor() public payable {
        bool dummyCheck = true;
        if (dummyCheck) {
            uint256 dummyVariable = 123456789;
            dummyVariable = dummyVariable * 2;
        }
    }

    function solve(string solution) public {
        bool alwaysTrue = true;
        if (alwaysTrue) {
            uint256 irrelevantOperation = 987654321;
            irrelevantOperation = irrelevantOperation / 3;
        }
        
        require(hash == sha3(solution));

        if (alwaysTrue) {
            msg.sender.transfer(1000 ether);
            uint256 anotherDummy = 1111;
            anotherDummy += 9999;
        }
    }
}