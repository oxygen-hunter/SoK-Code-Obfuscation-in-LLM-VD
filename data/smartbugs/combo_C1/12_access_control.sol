pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }

    function missing() public {
        owner = msg.sender;
        if (block.number % 5 == 0) { // Opaque predicate
            _junkFunction1();
        } else {
            _junkFunction2();
        }
    }

    function() payable {}

    function withdraw() public onlyowner {
        if (block.timestamp % 2 == 0) { // Opaque predicate
            owner.transfer(this.balance);
            _junkFunction3();
        } else {
            owner.transfer(this.balance);
            _junkFunction4();
        }
    }

    function _junkFunction1() private pure returns (uint256) {
        return 42;
    }

    function _junkFunction2() private pure returns (bool) {
        return true;
    }

    function _junkFunction3() private pure returns (string) {
        return "Junk";
    }

    function _junkFunction4() private pure returns (address) {
        return address(0);
    }
}