pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;  

    constructor() public payable {}  

    function () public payable {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                if (!(msg.value == 10 ether)) {
                    revert();
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                if (!(now != pastBlockTime)) {
                    revert();
                }
                dispatcher = 2;
            } else if (dispatcher == 2) {
                pastBlockTime = now;
                dispatcher = 3;
            } else if (dispatcher == 3) {
                if (now % 15 == 0) {
                    msg.sender.transfer(this.balance);
                }
                break;
            }
        }
    }
}