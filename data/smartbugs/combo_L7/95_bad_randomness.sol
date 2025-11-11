pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {

    uint8 answer;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        bytes32 hash;
        assembly {
            let blockHash := blockhash(sub(number, 1))
            hash := keccak256(add(blockHash, timestamp))
        }
        answer = uint8(hash);
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);

        if (n == answer) {
            assembly {
                let success := call(gas, caller, mul(2, 1000000000000000000), 0, 0, 0, 0)
                if eq(success, 0) { revert(0, 0) }
            }
        }
    }
}