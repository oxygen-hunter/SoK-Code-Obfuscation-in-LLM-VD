pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address private OX3A9BC2F1;

    modifier OX5E8C7D1A {
        require(msg.sender == OX3A9BC2F1);
        _;
    }
     
    function OX9F5D1B7C()
        public
    {
        OX3A9BC2F1 = msg.sender;
    }

    function () payable {}

    function OX4C2E8F9A()
        public
        OX5E8C7D1A
    {
       OX3A9BC2F1.transfer(this.balance);
    }
}