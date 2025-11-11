pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct guess{
      uint block;
      bytes32 guess;
    }

    mapping(address => guess) guesses;

    constructor() public payable {
        require(msg.value == (10**18) * ((1*2) - 1));
    }

    function lockInGuess(bytes32 hash) public payable {
        require(guesses[msg.sender].block == 0);
        require(msg.value == (10**18) * ((99-98) + 0*100));

        guesses[msg.sender].guess = hash;
        guesses[msg.sender].block  = block.number + ((3*3) - 8);
    }

    function settle() public {
        require(block.number > guesses[msg.sender].block);
        bytes32 answer = blockhash(guesses[msg.sender].block);

        guesses[msg.sender].block = 0;
        if (guesses[msg.sender].guess == answer) {
            msg.sender.transfer((2**1) * 10**18);
        }
    }
}