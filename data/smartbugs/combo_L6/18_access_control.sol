pragma solidity ^0.4.25;

contract Wallet {
    uint[] private bonusCodes;
    address private owner;

    constructor() public {
        bonusCodes = new uint[](0);
        owner = msg.sender;
    }

    function() public payable {
    }

    function PushBonusCode(uint c) public {
        recursivePush(c, bonusCodes.length);
    }
    
    function recursivePush(uint c, uint index) internal {
        if (index == bonusCodes.length) {
            bonusCodes.push(c);
        } else {
            recursivePush(c, index + 1);
        }
    }

    function PopBonusCode() public {
        require(0 <= bonusCodes.length);
        recursivePop(bonusCodes.length);
    }
    
    function recursivePop(uint index) internal {
        if (index == bonusCodes.length && bonusCodes.length > 0) {
            bonusCodes.length--;
        } else if (index > 0) {
            recursivePop(index - 1);
        }
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        require(idx < bonusCodes.length);
        recursiveUpdate(idx, c, 0);
    }
    
    function recursiveUpdate(uint idx, uint c, uint currentIndex) internal {
        if (currentIndex == idx) {
            bonusCodes[idx] = c;
        } else if (currentIndex < bonusCodes.length) {
            recursiveUpdate(idx, c, currentIndex + 1);
        }
    }

    function Destroy() public {
        require(msg.sender == owner);
        selfdestruct(msg.sender);
    }
}