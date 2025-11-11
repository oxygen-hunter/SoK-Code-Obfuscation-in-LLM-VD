pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct guess{
      bytes32 guess;
      uint block;
    }

    mapping(address => guess) guesses;

    uint private globalValue;

    constructor() public payable {
        globalValue = 1 ether;
        require(msg.value == globalValue);
    }

    function lockInGuess(bytes32 hash) public payable {
        require(guesses[msg.sender].block == 0);
        require(msg.value == globalValue);

        guess storage g = guesses[msg.sender];
        g.guess = hash;
        g.block = block.number + 1;
    }

    function settle() public {
        guess storage g = guesses[msg.sender];
        require(block.number > g.block);
        
        bytes32 answer = blockhash(g.block);

        g.block = 0;
        if (g.guess == answer) {
            msg.sender.transfer(2 ether);
        }
    }
}