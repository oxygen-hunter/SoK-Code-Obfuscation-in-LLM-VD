pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 private _answer;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        _answer = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);
        _transferIfGuessed(n, msg.sender);
    }

    function _transferIfGuessed(uint8 n, address sender) internal {
        if (n == _answer) {
            sender.transfer(2 ether);
        }
    }
}