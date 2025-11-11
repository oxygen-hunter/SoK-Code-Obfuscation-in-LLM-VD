pragma solidity ^0.4.25;

contract Wallet {
    address private owner;
    uint[] private bonusCodes;

    constructor() public {
        owner = msg.sender;
        bonusCodes = new uint[](0);
    }

    function () public payable {
    }

    function PushBonusCode(uint c) public {
        bonusCodes.push(c);
    }

    function PopBonusCode() public {
        require(0 <= bonusCodes.length);  
        bonusCodes.length--;  
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        require(idx < bonusCodes.length);
        bonusCodes[idx] = c;  
    }

    function Destroy() public {
        require(msg.sender == owner);
        selfdestruct(msg.sender);
    }
}