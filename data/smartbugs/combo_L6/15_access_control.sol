pragma solidity ^0.4.24;

contract Missing{
    address private o;

    modifier q {
        require(msg.sender==o);
        _;
    }

    function IamMissing()
        public
    {
        o = msg.sender;
    }

    function () payable {}

    function w()
        public
        q
    {
       o.transfer(this.balance);
    }
}