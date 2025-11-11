pragma solidity ^0.4.15;

contract Unprotected{
    modifier onlyowner {
        require(msg.sender==owner);
        _;
    }

    address private owner;

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