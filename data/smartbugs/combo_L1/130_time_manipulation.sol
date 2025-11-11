pragma solidity ^0.4.25;

contract OX7B4DF339 {
    uint public OXB2B9D8FE;  

    constructor() public payable {}  

    function () public payable {
        require(msg.value == 10 ether);  
        require(now != OXB2B9D8FE);  
        OXB2B9D8FE = now;
        if(now % 15 == 0) {  
            msg.sender.transfer(this.balance);
        }
    }
}