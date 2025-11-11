pragma solidity ^0.4.24;

contract Missing{
    struct O {
        address a;
    }
    O private o;

    modifier onlyowner {
        require(msg.sender == o.a);
        _;
    }

    function IamMissing()
        public
    {
        o.a = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       o.a.transfer(this.balance);
    }
}