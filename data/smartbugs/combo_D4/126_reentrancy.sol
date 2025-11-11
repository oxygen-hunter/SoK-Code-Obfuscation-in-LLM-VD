pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint[2] memory vars = [userBalances[msg.sender], 0];
        (bool success, ) = msg.sender.call.value(vars[0])("");
        require(success);
        userBalances[msg.sender] = vars[1];
    }
}