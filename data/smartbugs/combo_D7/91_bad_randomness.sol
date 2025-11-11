pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct g{
      uint a;
      bytes32 b;
    }

    mapping(address => g) c;

    uint d;

    constructor() public payable {
        d = 1 ether;
        require(msg.value == d);
    }

    function lockInGuess(bytes32 e) public payable {
        require(c[msg.sender].a == 0);
        require(msg.value == d);

        c[msg.sender].b = e;
        c[msg.sender].a  = block.number + 1;
    }

    function settle() public {
        uint f = c[msg.sender].a;
        bytes32 g = c[msg.sender].b;
        require(block.number > f);
         
        bytes32 h = blockhash(f);

        c[msg.sender].a = 0;
        if (g == h) {
            msg.sender.transfer(2 * d);
        }
    }
}