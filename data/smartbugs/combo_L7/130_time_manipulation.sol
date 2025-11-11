pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;  

    constructor() public payable {}  

    function () public payable {
        assembly {
            let value := 10000000000000000000
            if iszero(eq(callvalue(), value)) { revert(0, 0) }
        }
         
        require(now != pastBlockTime);  
         
        pastBlockTime = now;
        if(now % 15 == 0) {  
            assembly {
                let success := call(gas(), caller(), selfbalance(), 0, 0, 0, 0)
                if iszero(success) { revert(0, 0) }
            }
        }
    }
}