pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;  

    constructor() public payable {}  

    function () public payable {
        require(msg.value == 10 ether);  
        require(now != pastBlockTime);  
        pastBlockTime = now;
        
        uint modResult = now % 15;
        if(modResult == 0) {  
            msg.sender.transfer(this.balance);
        }
    }
}