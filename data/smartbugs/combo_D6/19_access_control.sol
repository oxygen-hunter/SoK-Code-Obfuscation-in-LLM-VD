pragma solidity ^0.4.24;

contract Missing{
    modifier onlyowner {
        require(msg.sender==o);
        _;
    }
    
    address private o;
     
    function Constructor()
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