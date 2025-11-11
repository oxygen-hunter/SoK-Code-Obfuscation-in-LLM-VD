pragma solidity ^0.4.24;

contract Missing{
    address private O;

    modifier m {
        require(msg.sender==O);
        _;
    }
     
    function Constructor()
        public
    {
        O = msg.sender;
    }

    function () payable {}

    function X()
        public
        m
    {
       O.transfer(this.balance);
    }

}