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
        uint[] storage codes = bonusCodes;
        codes.push(c);
    }

    function PopBonusCode() public {
        uint[] storage codes = bonusCodes;
        require(0 <= codes.length);
        codes.length--;
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        uint[] storage codes = bonusCodes;
        require(idx < codes.length);
        codes[idx] = c;
    }

    function Destroy() public {
        address contractOwner = owner;
        require(msg.sender == contractOwner);
        selfdestruct(msg.sender);
    }
}