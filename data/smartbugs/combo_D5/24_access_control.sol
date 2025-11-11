pragma solidity ^0.4.15;

contract Unprotected{
    address private a;

    modifier onlyowner {
        require(msg.sender==a);
        _;
    }

    function Unprotected()
        public
    {
        a = msg.sender;
    }

    function changeOwner(address _newOwner)
        public
    {
        a = _newOwner;
    }
}