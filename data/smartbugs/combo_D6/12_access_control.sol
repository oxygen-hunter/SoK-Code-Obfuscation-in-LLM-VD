pragma solidity ^0.4.24;

contract Missing{
    modifier onlyowner {
        require(msg.sender==owner);
        _;
    }
    
    address private owner;

    function missing()
        public
    {
        owner = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       owner.transfer(this.balance);
    }
}