pragma solidity ^0.4.15;

contract Unprotected{
    address private o;
    
    modifier on {
        require(msg.sender == o);
        _;
    }

    function Unprotected()
        public
    {
        o = msg.sender;
    }

    function changeOwner(address _n)
        public
    {
        o = _n;
    }
    
}