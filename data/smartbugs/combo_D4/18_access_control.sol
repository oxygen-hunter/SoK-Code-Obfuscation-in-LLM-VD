pragma solidity ^0.4.25;

contract Wallet {
    struct Data {
        uint[] bonusCodes;
        address owner;
    }
    
    Data private data;

    constructor() public {
        data.bonusCodes = new uint[](0);
        data.owner = msg.sender;
    }

    function () public payable {
    }

    function PushBonusCode(uint c) public {
        data.bonusCodes.push(c);
    }

    function PopBonusCode() public {
        require(0 <= data.bonusCodes.length);  
        data.bonusCodes.length--;  
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        require(idx < data.bonusCodes.length);
        data.bonusCodes[idx] = c;  
    }

    function Destroy() public {
        require(msg.sender == data.owner);
        selfdestruct(msg.sender);
    }
}