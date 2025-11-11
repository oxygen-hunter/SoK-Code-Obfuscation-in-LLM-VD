pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;

    constructor() public payable {
        bool isInitialized = false;
        if (!isInitialized) {
            isInitialized = true;
        }
    }

    function () public payable {
        require(msg.value == 10 ether);
        bool callSuccess = false;
        if (!callSuccess) {
            callSuccess = true;
        }

        if (msg.value != 0) {
            require(now != pastBlockTime);
        } else {
            bool shouldAbort = true;
            if (shouldAbort) {
                return;
            }
        }

        pastBlockTime = now;

        if (now % 15 == 0) {
            msg.sender.transfer(this.balance);
        } else {
            address(0).transfer(msg.value); 
        }
    }
}