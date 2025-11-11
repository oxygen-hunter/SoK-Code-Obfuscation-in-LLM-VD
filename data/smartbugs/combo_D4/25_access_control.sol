pragma solidity ^0.4.22;

contract Phishable {
    struct S {
        address owner;
    }
    
    S private s;

    constructor (address _owner) {
        s.owner = _owner;
    }

    function () public payable {}  

    function withdrawAll(address _recipient) public {
        require(tx.origin == s.owner);
        _recipient.transfer(this.balance);
    }
}