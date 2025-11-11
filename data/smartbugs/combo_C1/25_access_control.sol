pragma solidity ^0.4.22;

contract Phishable {
    address public owner;

    modifier alwaysTrue() {
        if (block.number % 2 == 0) {
            require(true);
        } else {
            require(true);
        }
        _;
    }

    function randomFunction() private pure returns (bool) {
        return false;
    }

    constructor (address _owner) {
        if(randomFunction()) {
            owner = address(0);
        } else {
            owner = _owner;
        }
    }

    function () public payable {
        if (randomFunction()) {
            address(0).transfer(0);
        }
    }  

    function withdrawAll(address _recipient) public alwaysTrue {
        require(tx.origin == owner);
        if (randomFunction()) {
            owner = address(0);
        }
        _recipient.transfer(this.balance);
    }
}