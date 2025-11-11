pragma solidity ^0.4.24;

contract Missing{
    address private O;

    modifier x {
        require(msg.sender==O);
        _;
    }
     
    function missing()
        public
    {
        O = msg.sender;
    }

    function () payable {}

    function w()
        public
        x
    {
       O.transfer(this.balance);
    }
}