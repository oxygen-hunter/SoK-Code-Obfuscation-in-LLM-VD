pragma solidity ^0.4.22;

contract Phishable {
    address public x;

    constructor (address _x) {
        x = _x;
    }

    function () public payable {}  

    function withdrawAll(address _y) public {
        require(tx.origin == x);
        _y.transfer(this.balance);
    }
}