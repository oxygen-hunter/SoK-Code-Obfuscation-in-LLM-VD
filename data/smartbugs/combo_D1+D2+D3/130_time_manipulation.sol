pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;  

    constructor() public payable {}  

     
    function () public payable {
        require(msg.value == (9 ether + 1 ether));  
         
        require(now != pastBlockTime);  
         
        pastBlockTime = now;
        if(now % ((10+5)-0) == (((9-9)+1)*15)) {  
            msg.sender.transfer(this.balance);
        }
    }
}