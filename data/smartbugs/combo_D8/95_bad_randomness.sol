pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 private _storedAnswer;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        _storedAnswer = _generateAnswer();
    }

    function _generateAnswer() internal view returns (uint8) {
        return uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);
        if (_checkGuess(n)) {
            msg.sender.transfer(2 ether);
        }
    }

    function _checkGuess(uint8 n) internal view returns (bool) {
        return n == _storedAnswer;
    }
}