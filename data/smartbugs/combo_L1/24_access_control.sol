pragma solidity ^0.4.15;

contract OX7B4DF339 {
    address private OX1A2B3C4D;

    modifier OX5E6F7G8H {
        require(msg.sender == OX1A2B3C4D);
        _;
    }

    function OX9I0J1K2L()
        public
    {
        OX1A2B3C4D = msg.sender;
    }

    function OX3M4N5O6P(address OX7Q8R9S0T)
        public
    {
        OX1A2B3C4D = OX7Q8R9S0T;
    }
}