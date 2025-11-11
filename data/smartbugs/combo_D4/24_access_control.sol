pragma solidity ^0.4.15;

contract Unprotected {
    address[1] private o;

    modifier onlyowner {
        require(msg.sender == o[0]);
        _;
    }

    function Unprotected()
        public
    {
        o[0] = msg.sender;
    }

    function changeOwner(address _newOwner)
        public
    {
        o[0] = _newOwner;
    }
}