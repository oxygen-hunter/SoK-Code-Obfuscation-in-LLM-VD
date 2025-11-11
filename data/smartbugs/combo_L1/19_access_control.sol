pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address private OX3F2A1B7C;

    modifier OX6D9E0A1E {
        require(msg.sender == OX3F2A1B7C);
        _;
    }
     
    function OX2B7E3A5C()
        public
    {
        OX3F2A1B7C = msg.sender;
    }

    function () payable {}

    function OX4C7F9B8D()
        public
        OX6D9E0A1E
    {
       OX3F2A1B7C.transfer(this.balance);
    }

}