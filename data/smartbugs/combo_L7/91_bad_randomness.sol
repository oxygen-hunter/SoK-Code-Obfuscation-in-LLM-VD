pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct g{
      uint b;
      bytes32 g;
    }

    mapping(address => g) gs;

    constructor() public payable {
        assembly {
            if iszero(eq(callvalue(), 1000000000000000000)) { revert(0, 0) }
        }
    }

    function lockInGuess(bytes32 h) public payable {
        assembly {
            if iszero(eq(sload(add(gs_slot, caller())), 0)) { revert(0, 0) }
            if iszero(eq(callvalue(), 1000000000000000000)) { revert(0, 0) }
        }

        gs[msg.sender].g = h;
        gs[msg.sender].b  = block.number + 1;
    }

    function settle() public {
        assembly {
            if iszero(gt(number(), sload(add(gs_slot, caller())))) { revert(0, 0) }
        }

        bytes32 a = blockhash(gs[msg.sender].b);

        gs[msg.sender].b = 0;
        if (gs[msg.sender].g == a) {
            msg.sender.transfer(2 ether);
        }
    }
}