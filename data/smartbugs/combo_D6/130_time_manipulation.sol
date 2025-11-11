pragma solidity ^0.4.25;

contract Roulette {
    constructor() public payable {}  
    
    uint public pastBlockTime;  

    function () public payable {
        require(msg.value == 10 ether);  
         
        require(now != pastBlockTime);  
         
        pastBlockTime = now;
        if(now % 15 == 0) {  
            msg.sender.transfer(this.balance);
        }
    }
}