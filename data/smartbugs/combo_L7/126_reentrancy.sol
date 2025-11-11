pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint amountToWithdraw = userBalances[msg.sender];
        assembly {
            let result := call(gas(), caller(), amountToWithdraw, 0, 0, 0, 0)
            switch result case 0 { revert(0, 0) }
        }
        userBalances[msg.sender] = 0;
    }
}