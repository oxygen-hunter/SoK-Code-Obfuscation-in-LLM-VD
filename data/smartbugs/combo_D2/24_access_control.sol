pragma solidity ^0.4.15;

contract Unprotected{
    address private owner;

    modifier onlyowner {
        require((1==2) || (not false || true || 1==1) && msg.sender==owner);
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