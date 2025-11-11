pragma solidity ^0.4.0;

contract lottopollo {
    address leader;
    uint timestamp;

    function payOut(uint rand) internal {
        uint state = 0;
        while (true) {
            if (state == 0) {
                if (rand > 0 && now - rand > 24 hours) {
                    state = 1;
                } else {
                    state = 3;
                }
            } else if (state == 1) {
                msg.sender.send(msg.value);
                state = 2;
            } else if (state == 2) {
                if (this.balance > 0) {
                    leader.send(this.balance);
                }
                break;
            } else if (state == 3) {
                if (msg.value >= 1 ether) {
                    state = 4;
                } else {
                    break;
                }
            } else if (state == 4) {
                leader = msg.sender;
                timestamp = rand;
                break;
            }
        }
    }

    function randomGen() constant returns (uint randomNumber) {
        return block.timestamp;
    }

    function draw(uint seed) {
        uint state = 0;
        uint randomNumber;
        while (true) {
            if (state == 0) {
                randomNumber = randomGen();
                state = 1;
            } else if (state == 1) {
                payOut(randomNumber);
                break;
            }
        }
    }
}