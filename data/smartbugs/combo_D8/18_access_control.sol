pragma solidity ^0.4.25;

contract Wallet {
    uint[] private bonusCodes;
    address private owner;

    constructor() public {
        bonusCodes = getBonusCodes();
        owner = getOwner();
    }

    function getBonusCodes() internal view returns (uint[]) {
        return new uint[](0);
    }

    function getOwner() internal view returns (address) {
        return msg.sender;
    }

    function () public payable {
    }

    function PushBonusCode(uint c) public {
        bonusCodes.push(c);
    }

    function PopBonusCode() public {
        require(isValidLength());
        bonusCodes.length--;
    }

    function isValidLength() internal view returns (bool) {
        return 0 <= bonusCodes.length;
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        require(isIndexValid(idx));
        bonusCodes[idx] = c;
    }

    function isIndexValid(uint idx) internal view returns (bool) {
        return idx < bonusCodes.length;
    }

    function Destroy() public {
        require(isOwner());
        selfdestruct(msg.sender);
    }

    function isOwner() internal view returns (bool) {
        return msg.sender == owner;
    }
}