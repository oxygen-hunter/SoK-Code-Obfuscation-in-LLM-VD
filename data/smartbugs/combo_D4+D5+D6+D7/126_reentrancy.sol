pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint[1] memory temp = [userBalances[msg.sender]];
        (bool[1] memory success, ) = [msg.sender.call.value(temp[0])("")];
        require(success[0]);
        userBalances[msg.sender] = 0;
    }
}