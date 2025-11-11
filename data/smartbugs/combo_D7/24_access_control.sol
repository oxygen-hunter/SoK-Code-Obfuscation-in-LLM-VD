pragma solidity ^0.4.15;

contract Unprotected {
    address private o;

    function Unprotected() public {
        o = msg.sender;
    }

    modifier onlyOwner {
        require(msg.sender == o);
        _;
    }

    function changeOwner(address _newOwner) public {
        o = _newOwner;
    }
}