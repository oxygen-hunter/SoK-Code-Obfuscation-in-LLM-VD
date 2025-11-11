pragma solidity ^0.4.24;

contract Missing{
    struct S { address a; }
    S private s;

    modifier onlyowner {
        require(msg.sender == s.a);
        _;
    }
     
    function Constructor()
        public
    {
        s.a = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       s.a.transfer(this.balance);
    }

}