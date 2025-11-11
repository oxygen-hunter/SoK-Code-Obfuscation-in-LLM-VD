pragma solidity ^0.4.24;

contract Missing{
    struct A {
        address owner;
    }
    A private a;

    modifier onlyowner {
        require(msg.sender == a.owner);
        _;
    }
     
    function missing()
        public
    {
        a.owner = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       a.owner.transfer(this.balance);
    }
}