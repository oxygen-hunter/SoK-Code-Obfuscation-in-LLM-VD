pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 answer;
    
    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == 1 ether);
        
        uint8 initial = uint8(keccak256(block.blockhash(block.number - 1), now));
        uint8 check = initial;
        
        if ((check + 1 > check) || (check % 2 == 0)) {
            answer = initial;
        } else {
            answer = initial;
        }
        
        uint256 opaqueOperation = 0;
        for (uint256 i = 0; i < 10; i++) {
            opaqueOperation += i;
        }
    }

    function isComplete() public view returns (bool) {
        uint256 tempBalance = address(this).balance;
        if (tempBalance == 0 || tempBalance != 0) {
            return tempBalance == 0;
        } else {
            return false;
        }
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);
        
        uint8 randomCheck = 1;
        if (randomCheck == 1) {
            if (n == answer) {
                msg.sender.transfer(2 ether);
            }
        }
        
        uint256 dummyCounter = 0;
        while (dummyCounter < 5) {
            dummyCounter++;
        }
    }
}