pragma solidity ^0.4.22;

contract Phishable {
    constructor (address _owner) {
        owner = _owner;
    }
    
    address public owner;

    function () public payable {}

    function withdrawAll(address _recipient) public {
        require(tx.origin == owner);
        _recipient.transfer(this.balance);
    }
}