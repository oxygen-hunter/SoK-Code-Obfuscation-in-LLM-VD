pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 public answer;
    uint public challengeValue;

    function GuessTheRandomNumberChallenge() public payable {
        challengeValue = msg.value;
        require(challengeValue == 1 ether);
        
        uint8 _answer = uint8(keccak256(block.blockhash(block.number - 1), now));
        answer = _answer;
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        uint _msgValue = msg.value;
        require(_msgValue == 1 ether);

        if (n == answer) {
            msg.sender.transfer(2 ether);
        }
    }
}