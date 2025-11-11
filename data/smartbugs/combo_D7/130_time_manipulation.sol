pragma solidity ^0.4.25;

contract Roulette {
    uint public pbt;  

    constructor() public payable {}  

    function () public payable {
        uint _n = now;
        require(msg.value == 10 ether);  
        require(_n != pbt);  
        pbt = _n;
        if(_n % 15 == 0) {  
            msg.sender.transfer(this.balance);
        }
    }
}