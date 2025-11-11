pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct g{
      uint b;
      bytes32 g;
    }

    mapping(address => g) gs;

    constructor() public payable {
        require(msg.value == 1 ether);
    }

    function lockInGuess(bytes32 h) public payable {
        require(gs[msg.sender].b == 0);
        require(msg.value == 1 ether);

        gs[msg.sender].g = h;
        gs[msg.sender].b  = block.number + 1;
    }

    function settle() public {
        require(block.number > gs[msg.sender].b);
         
        bytes32 a = blockhash(gs[msg.sender].b);

        gs[msg.sender].b = 0;
        if (gs[msg.sender].g == a) {
            msg.sender.transfer(2 ether);
        }
    }
}