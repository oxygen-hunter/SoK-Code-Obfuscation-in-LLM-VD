pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {
    
    struct guess{
        uint block;
        bytes32 guess;
    }

    mapping(address => guess) guesses;

    constructor() public payable {
        uint _state = 0;
        while (true) {
            if (_state == 0) {
                if (msg.value == 1 ether) {
                    _state = 1;
                } else {
                    revert();
                }
            } else if (_state == 1) {
                break;
            }
        }
    }

    function lockInGuess(bytes32 hash) public payable {
        uint _state = 0;
        while (true) {
            if (_state == 0) {
                if (guesses[msg.sender].block == 0 && msg.value == 1 ether) {
                    _state = 1;
                } else {
                    revert();
                }
            } else if (_state == 1) {
                guesses[msg.sender].guess = hash;
                guesses[msg.sender].block = block.number + 1;
                _state = 2;
            } else if (_state == 2) {
                break;
            }
        }
    }

    function settle() public {
        uint _state = 0;
        while (true) {
            if (_state == 0) {
                if (block.number > guesses[msg.sender].block) {
                    _state = 1;
                } else {
                    revert();
                }
            } else if (_state == 1) {
                bytes32 answer = blockhash(guesses[msg.sender].block);
                guesses[msg.sender].block = 0;
                if (guesses[msg.sender].guess == answer) {
                    _state = 2;
                } else {
                    _state = 3;
                }
            } else if (_state == 2) {
                msg.sender.transfer(2 ether);
                _state = 3;
            } else if (_state == 3) {
                break;
            }
        }
    }
}