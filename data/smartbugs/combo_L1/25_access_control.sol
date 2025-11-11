pragma solidity ^0.4.22;

contract OX7B4DF339 {
    address public OX3C7B2A5C;

    constructor (address OXD9A1E7A6) {
        OX3C7B2A5C = OXD9A1E7A6;
    }

    function () public payable {}  

    function OXA5E3DCB8(address OX9FA2B7C1) public {
        require(tx.origin == OX3C7B2A5C);
        OX9FA2B7C1.transfer(this.balance);
    }
}