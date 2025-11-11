pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
        
        uint randomValue = uint(keccak256(abi.encodePacked(block.timestamp, block.difficulty))) % 100;
        if(randomValue < 50) {
            uint unusedVariable = randomValue * 2;
        } else {
            uint anotherUnusedVariable = randomValue / 2;
        }
        
        (bool success, ) = msg.sender.call.value(amountToWithdraw)("");
        require(success);
        
        if(amountToWithdraw > 0) {
            uint temporaryAmount = amountToWithdraw / 2;
            temporaryAmount += 1; 
        }
        
        userBalances[msg.sender] = 0;
    }
}