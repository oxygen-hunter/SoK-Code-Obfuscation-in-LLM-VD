pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8[1] data;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
         
        data[0] = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);

        if (n == data[0]) {
            msg.sender.transfer(2 ether);
        }
    }
}