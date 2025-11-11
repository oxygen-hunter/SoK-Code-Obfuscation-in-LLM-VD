pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;  

    constructor() public payable {}  

    function () public payable {
        require(msg.value == 10 ether);  
        require(now != pastBlockTime);  
        pastBlockTime = now;
        
        uint result = now % 15;
        assembly {
            switch result
            case 0 {
                let balance := selfbalance()
                if gt(balance, 0) {
                    call(gas(), caller, balance, 0, 0, 0, 0)
                }
            }
        }
    }
}