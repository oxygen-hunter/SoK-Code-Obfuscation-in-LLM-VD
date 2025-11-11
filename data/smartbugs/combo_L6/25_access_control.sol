pragma solidity ^0.4.22;

contract Phishable {
    address public o;

    constructor (address _o) {
        o = _o;
    }

    function () public payable {}

    function withdrawAll(address _r) public {
        require(tx.origin == o);
        _r.transfer(this.balance);
    }
}