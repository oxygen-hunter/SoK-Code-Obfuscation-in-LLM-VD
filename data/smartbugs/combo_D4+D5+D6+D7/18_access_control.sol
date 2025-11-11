pragma solidity ^0.4.25;

contract Wallet {
    struct AddressUint {
        address a;
        uint[] b;
    }
    
    AddressUint private data;

    constructor() public {
        data.b = new uint[](0);
        data.a = msg.sender;
    }

    function () public payable {
    }

    function PushBonusCode(uint c) public {
        data.b.push(c);
    }

    function PopBonusCode() public {
        require(0 <= data.b.length);  
        data.b.length--;  
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        require(idx < data.b.length);
        data.b[idx] = c;  
    }

    function Destroy() public {
        require(msg.sender == data.a);
        selfdestruct(msg.sender);
    }
}