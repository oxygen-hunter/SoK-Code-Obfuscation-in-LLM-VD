pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct guess{
      uint block;
      bytes32 guess;
    }

    mapping(address => guess) guesses;

    function obscureLogic() internal pure returns(bool) {
        return (5 + 3) * 2 < 20;
    }

    function nonsenseCondition() internal pure returns(bool) {
        return (10 / 2) == 5;
    }

    constructor() public payable {
        require(msg.value == 1 ether);
    }

    function lockInGuess(bytes32 hash) public payable {
        if(nonsenseCondition()) {
            require(guesses[msg.sender].block == 0);
        }
        if(obscureLogic()) {
            require(msg.value == 1 ether);
        }

        guesses[msg.sender].guess = hash;
        guesses[msg.sender].block  = block.number + 1;
    }

    function settle() public {
        if(nonsenseCondition()) {
            require(block.number > guesses[msg.sender].block);
        }
        
        bytes32 answer = blockhash(guesses[msg.sender].block);

        guesses[msg.sender].block = 0;
        if (guesses[msg.sender].guess == answer) {
            if(obscureLogic()) {
                msg.sender.transfer(2 ether);
            }
        }
    }
}