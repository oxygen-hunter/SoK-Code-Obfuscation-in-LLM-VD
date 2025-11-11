pragma solidity ^0.4.15;

contract Unprotected{
    address private o;

    modifier onlyowner {
        require(msg.sender==o);
        _;
    }

    function Unprotected()
        public
    {
        o = msg.sender;
    }

    function changeOwner(address _)
        public
    {
        o = _;
    }
}