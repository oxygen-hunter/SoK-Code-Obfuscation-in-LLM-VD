pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    mapping(address => uint) blockNumbers;
    mapping(address => bytes32) guesses;

    constructor() public payable {
        require(msg.value == 1 ether);
    }

    function lockInGuess(bytes32 hash) public payable {
        require(blockNumbers[msg.sender] == 0);
        require(msg.value == 1 ether);

        guesses[msg.sender] = hash;
        blockNumbers[msg.sender] = block.number + 1;
    }

    function settle() public {
        require(block.number > blockNumbers[msg.sender]);
        
        bytes32 answer = blockhash(blockNumbers[msg.sender]);

        blockNumbers[msg.sender] = 0;
        if (guesses[msg.sender] == answer) {
            msg.sender.transfer(2 ether);
        }
    }
}