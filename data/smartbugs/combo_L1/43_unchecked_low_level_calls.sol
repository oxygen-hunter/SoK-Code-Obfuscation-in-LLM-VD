pragma solidity ^0.4.18;

contract OX7B4DF339 {

    bool public OX1A2B3C4D = false;
    address public OX5E6F7G8H;
    uint public OX9I0J1K2L;

    function OXM3N4O5P() public {
        require(!OX1A2B3C4D);
        
        OX5E6F7G8H.send(OX9I0J1K2L);
        OX1A2B3C4D = true;
    }

    function OXQ6R7S8T() public {
        require(OX1A2B3C4D);
        
        msg.sender.send(this.balance);
    }
}