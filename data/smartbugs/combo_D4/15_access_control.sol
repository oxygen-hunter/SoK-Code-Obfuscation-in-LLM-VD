pragma solidity ^0.4.24;

contract Missing {
    struct S { address o; }
    S private s;

    modifier onlyowner {
        require(msg.sender == s.o);
        _;
    }

    function IamMissing()
        public
    {
        s.o = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
        s.o.transfer(this.balance);
    }
}