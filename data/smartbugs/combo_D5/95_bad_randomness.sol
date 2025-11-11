pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 a;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        
        a = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 b) public payable {
        require(msg.value == 1 ether);

        if (b == a) {
            msg.sender.transfer(2 ether);
        }
    }
}