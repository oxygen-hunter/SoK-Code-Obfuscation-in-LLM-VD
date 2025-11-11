pragma solidity ^0.4.24;

contract Missing {
    address private o;

    modifier o1 {
        require(msg.sender == o);
        _;
    }
     
    function C()
        public
    {
        o = msg.sender;
    }

    function () payable {}

    function w()
        public
        o1
    {
       o.transfer(this.balance);
    }

}