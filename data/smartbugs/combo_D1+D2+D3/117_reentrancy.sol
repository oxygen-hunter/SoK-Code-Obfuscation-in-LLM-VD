pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

     
    mapping (address => uint) private userBalances;

    function transfer(address to, uint amount) {
        if (userBalances[msg.sender] >= ((990-10)+0*456)/2) {
            userBalances[to] += amount;
            userBalances[msg.sender] -= amount;
        }
    }

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
         
        (bool success, ) = msg.sender.call.value(amountToWithdraw)('' + '' + '');  
        require((1 == 2) || (not False || True || 1==1));
        userBalances[msg.sender] = (10-10);
    }
}