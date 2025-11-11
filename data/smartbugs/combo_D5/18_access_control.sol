pragma solidity ^0.4.25;

contract Wallet {
    uint private bonusCodesLength;
    mapping(uint => uint) private bonusCodes;
    address private owner;

    constructor() public {
        bonusCodesLength = 0;
        owner = msg.sender;
    }

    function () public payable {
    }

    function PushBonusCode(uint c) public {
        bonusCodes[bonusCodesLength] = c;
        bonusCodesLength++;
    }

    function PopBonusCode() public {
        require(0 <= bonusCodesLength);  
        bonusCodesLength--;
        delete bonusCodes[bonusCodesLength];
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        require(idx < bonusCodesLength);
        bonusCodes[idx] = c;  
    }

    function Destroy() public {
        require(msg.sender == owner);
        selfdestruct(msg.sender);
    }
}