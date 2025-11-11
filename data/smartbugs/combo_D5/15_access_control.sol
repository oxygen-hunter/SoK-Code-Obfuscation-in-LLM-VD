pragma solidity ^0.4.24;

contract Missing{
    address private o;
    modifier m {
        require(msg.sender==o);
        _;
    }

    function i()
        public
    {
        o = msg.sender;
    }

    function () payable {}

    function w()
        public
        m
    {
       o.transfer(this.balance);
    }
}