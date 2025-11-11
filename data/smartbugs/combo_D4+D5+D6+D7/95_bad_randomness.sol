pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    struct Data { 
        uint8 a; 
    }
    Data private data;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
         
        data.a = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);

        if (n == data.a) {
            msg.sender.transfer(2 ether);
        }
    }
}