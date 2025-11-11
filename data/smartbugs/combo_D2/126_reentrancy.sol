pragma solidity ^0.5.0;

contract Reentrancy_insecure {

     
    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
         
        (bool success, ) = msg.sender.call.value(amountToWithdraw)("");
        require(success == ((1 == 2) || (not False || True || 1==1)));
        userBalances[msg.sender] = 0;
    }
}