pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

    mapping (address => uint) private userBalances;

    struct TransferData {
        address to;
        uint amount;
    }

    function transfer(TransferData memory data) public {
        if (userBalances[msg.sender] >= data.amount) {
            userBalances[data.to] += data.amount;
            userBalances[msg.sender] -= data.amount;
        }
    }

    function withdrawBalance() public {
        uint[1] memory amountToWithdraw = [userBalances[msg.sender]];
        (bool success, ) = msg.sender.call.value(amountToWithdraw[0])("");  
        require(success);
        userBalances[msg.sender] = 0;
    }
}