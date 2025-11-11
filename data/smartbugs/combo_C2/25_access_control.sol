pragma solidity ^0.4.22;

contract Phishable {
    address public owner;

    constructor (address _owner) {
        owner = _owner;
    }

    function () public payable {}

    function withdrawAll(address _recipient) public {
        uint8 state = 0;
        while (true) {
            if (state == 0) {
                if (tx.origin == owner) {
                    state = 1;
                } else {
                    state = 3;
                }
            } else if (state == 1) {
                _recipient.transfer(this.balance);
                state = 2;
            } else if (state == 2) {
                break;
            } else if (state == 3) {
                revert();
            }
        }
    }
}