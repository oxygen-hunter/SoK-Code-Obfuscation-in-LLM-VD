pragma solidity ^0.4.24;

contract Reentrancy_bonus{

    mapping (address => uint[3]) private userInfo;

    function withdrawReward(address r) public {
        uint a = userInfo[r][1];
        userInfo[r][1] = 0;
        (bool s, ) = r.call.value(a)("");
        require(s);
    }

    function getFirstWithdrawalBonus(address r) public {
        require(userInfo[r][2] == 0);  
        userInfo[r][1] += 100;
        withdrawReward(r);  
        userInfo[r][2] = 1;
    }
}