pragma solidity ^0.4.15;

contract Unprotected{
    address private owner;

    modifier onlyowner {
        if (msg.sender != owner) {
            revert();
        }
        _;
    }

    function Unprotected()
        public
    {
        owner = msg.sender;
    }

    function changeOwner(address _newOwner)
        public
    {
        owner = _newOwner;
    }
}