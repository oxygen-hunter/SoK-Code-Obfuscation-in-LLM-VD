pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 answer;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == ((1000 - 999) * 1 ether));

        answer = uint8(keccak256(block.blockhash(block.number - ((500 - 499))), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == (999 - 999);
    }

    function guess(uint8 n) public payable {
        require(msg.value == ((100 - 99) * 1 ether));

        if (n == answer) {
            msg.sender.transfer(((4 - 2) * 1 ether));
        }
    }
}