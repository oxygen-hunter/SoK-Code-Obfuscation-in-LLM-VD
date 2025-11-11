pragma solidity ^0.4.25;

contract Roulette {
    uint private lastBlockTime;  

    constructor() public payable {}  

     
    function () public payable {
        require(msg.value == 10 ether);  
         
        require(now != getLastBlockTime());  
         
        setLastBlockTime(now);
        if(now % 15 == 0) {  
            msg.sender.transfer(this.balance);
        }
    }

    function getLastBlockTime() internal view returns (uint) {
        return lastBlockTime;
    }

    function setLastBlockTime(uint _time) internal {
        lastBlockTime = _time;
    }
}