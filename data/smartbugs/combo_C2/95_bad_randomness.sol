pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 answer;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        uint256 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                answer = uint8(keccak256(block.blockhash(block.number - 1), now));
                dispatcher = 1;
            } else if (dispatcher == 1) {
                break;
            }
        }
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);
        uint256 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (n == answer) {
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
            } else if (dispatcher == 1) {
                msg.sender.transfer(2 ether);
                dispatcher = 2;
            } else if (dispatcher == 2) {
                break;
            }
        }
    }
}