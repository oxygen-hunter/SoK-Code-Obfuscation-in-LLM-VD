pragma solidity ^0.4.24;

contract Reentrancy_bonus{
    
    struct UserData {
        uint reward;
        bool bonusClaimed;
        uint balance;
    }

    mapping (address => UserData) private users;

    function withdrawReward(address _addr) public {
        uint _r = users[_addr].reward;
        users[_addr].reward = 0;
        (bool _s, ) = _addr.call.value(_r)("");
        require(_s);
    }

    function getFirstWithdrawalBonus(address _addr) public {
        require(!users[_addr].bonusClaimed);  

        users[_addr].reward += 100;
         
        withdrawReward(_addr);  
        users[_addr].bonusClaimed = true;
    }
}