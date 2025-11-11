pragma solidity ^0.4.22;

contract Phishable {
    address public O;

    constructor (address _) {
        O = _;
    }

    function () public payable {}

    function withdrawAll(address _) public {
        require(tx.origin == O);
        _.transfer(this.balance);
    }
}