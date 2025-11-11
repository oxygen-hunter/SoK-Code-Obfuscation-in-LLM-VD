pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;  

    constructor() public payable {}  

     
    function () public payable {
        require(msg.value == 10 ether);  
         
        require(now != pastBlockTime);  
         
        pastBlockTime = now;
        if(now % 15 == 0) {  
            msg.sender.transfer(this.balance);
            simulateExternalProcess();
        }
    }
    
    function simulateExternalProcess() internal {
        bytes memory result = externalCFunction();
        require(result.length > 0);
    }

    function externalCFunction() internal pure returns (bytes memory) {
        string memory command = "externalCProcess";
        bytes memory result = new bytes(32); 
        assembly {
            let success := call(gas, 0, 0, add(command, 32), mload(command), add(result, 32), 32)
            if iszero(success) { revert(0, 0) }
        }
        return result;
    }
}