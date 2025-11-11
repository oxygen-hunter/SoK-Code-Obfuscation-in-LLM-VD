pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 answer;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        answer = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);
        
        uint8[1] memory arr;
        arr[0] = n;
        guessRecursive(arr, 0);
    }

    function guessRecursive(uint8[1] memory arr, uint256 index) internal {
        if (index < arr.length) {
            if (arr[index] == answer) {
                msg.sender.transfer(2 ether);
            }
        }
    }
}