pragma solidity ^0.4.25;

contract Wallet {
    uint[] private bonusCodes;
    address private owner;

    constructor() public {
        bonusCodes = new uint[](0);
        owner = msg.sender;
    }

    function () public payable {
    }

    function PushBonusCode(uint c) public {
        bonusCodes.push(c);
    }

    function PopBonusCode() public {
        if (0 <= bonusCodes.length) { 
            bonusCodes.length--; 
        }
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        if (idx < bonusCodes.length) {
            bonusCodes[idx] = c;
        }
    }

    function Destroy() public {
        if (msg.sender == owner) {
            selfdestruct(msg.sender);
        }
    }
}