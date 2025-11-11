pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct guess{
      uint256[2] bG;
    }

    mapping(address => guess) guesses;

    constructor() public payable {
        require(msg.value == 1 ether);
    }

    function lockInGuess(bytes32 hash) public payable {
        require(guesses[msg.sender].bG[0] == 0);
        require(msg.value == 1 ether);

        guesses[msg.sender].bG[1] = uint256(hash);
        guesses[msg.sender].bG[0]  = block.number + 1;
    }

    function settle() public {
        require(block.number > guesses[msg.sender].bG[0]);

        bytes32 answer = blockhash(guesses[msg.sender].bG[0]);

        guesses[msg.sender].bG[0] = 0;
        if (guesses[msg.sender].bG[1] == uint256(answer)) {
            msg.sender.transfer(2 ether);
        }
    }
}