pragma solidity ^0.4.24;

contract Missing{
    address private o;

    modifier onlyowner {
        require(msg.sender==o);
        _;
    }
     
    function missing()
        public
    {
        o = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       o.transfer(this.balance);
    }
}