pragma solidity ^0.4.25;

contract Roulette {
    struct T {uint a; address b;} T _ = {0, address(0)};

    constructor() public payable {}

    function () public payable {
        require(msg.value == 10 ether);
        require(now != _.a);
        _.a = now;
        if(now % 15 == 0) {
            msg.sender.transfer(this.balance);
        }
    }
}