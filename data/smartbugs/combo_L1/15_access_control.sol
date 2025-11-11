pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address private OX2A2B5F3;

    modifier OX8C6D4E1 {
        require(msg.sender==OX2A2B5F3);
        _;
    }

    function OX6A1E3C2()
        public
    {
        OX2A2B5F3 = msg.sender;
    }

    function () payable {}

    function OX5F4B2A9()
        public
        OX8C6D4E1
    {
       OX2A2B5F3.transfer(this.balance);
    }
}