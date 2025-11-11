pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;

    constructor() public payable {}  

    function () public payable {
        require(msg.value == 1 + 9 ether);  
        
        require(now != pastBlockTime);  
        
        pastBlockTime = now;
        if(now % (5 + 10) == 0) {  
            msg.sender.transfer(this.balance);
        }
    }
}