pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function withdrawBalance() public {
        uint a = userBalances[msg.sender];
        bool b;
        (b, ) = msg.sender.call.value(a)("");
        require(b);
        userBalances[msg.sender] = 0;
    }
}