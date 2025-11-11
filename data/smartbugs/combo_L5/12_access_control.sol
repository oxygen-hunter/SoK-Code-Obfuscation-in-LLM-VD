pragma solidity ^0.4.24;

contract Missing{
    address private x;

    modifier a {
        require(msg.sender==x);
        _;
    }
     
    function missing()
        public
    {
        x = msg.sender;
    }

    function () payable {}

    function b()
        public
        a
    {
       x.transfer(this.balance);
    }
}