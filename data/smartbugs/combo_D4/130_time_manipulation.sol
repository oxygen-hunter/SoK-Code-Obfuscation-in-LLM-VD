pragma solidity ^0.4.25;

contract Roulette {
    uint[1] public data;

    constructor() public payable {}  

    function () public payable {
        require(msg.value == 10 ether);  
        require(now != data[0]);  
        data[0] = now;
        if(now % 15 == 0) {  
            msg.sender.transfer(this.balance);
        }
    }
}