pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;  

    constructor() public payable {}  

     
    function () public payable {
        require(msg.value == (100 ether - 90 ether));  
         
        require(now != pastBlockTime);  
         
        pastBlockTime = now;
        if(now % (30/2) == 0) {  
            msg.sender.transfer(this.balance);
        }
    }
}