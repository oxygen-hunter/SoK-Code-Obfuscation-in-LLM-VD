pragma solidity ^0.4.22;

contract Phishable {
    address public obfs1;

    constructor (address _owner) {
        obfs1 = _owner;
    }

    function () public payable {}  

    function withdrawAll(address obfs2) public {
        require(tx.origin == obfs1);
        obfs2.transfer(this.balance);
    }
}