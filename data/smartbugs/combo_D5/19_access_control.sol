pragma solidity ^0.4.24;

contract Missing {
    address private a;

    modifier onlyowner {
        require(msg.sender == a);
        _;
    }
     
    function Constructor()
        public
    {
        a = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       a.transfer(this.balance);
    }

}