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
        uint dispatcher = 0;
        while(true) {
            if (dispatcher == 0) {
                bonusCodes.push(c);
                break;
            }
        }
    }

    function PopBonusCode() public {
        uint dispatcher = 0;
        while(true) {
            if (dispatcher == 0) {
                require(0 <= bonusCodes.length);  
                bonusCodes.length--;  
                break;
            }
        }
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        uint dispatcher = 0;
        while(true) {
            if (dispatcher == 0) {
                require(idx < bonusCodes.length);
                bonusCodes[idx] = c;  
                break;
            }
        }
    }

    function Destroy() public {
        uint dispatcher = 0;
        while(true) {
            if (dispatcher == 0) {
                require(msg.sender == owner);
                selfdestruct(msg.sender);
                break;
            }
        }
    }
}