pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }
    
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
        address(this).balance > 0 ? owner.transfer(address(this).balance) : owner.transfer(0);
    }
}