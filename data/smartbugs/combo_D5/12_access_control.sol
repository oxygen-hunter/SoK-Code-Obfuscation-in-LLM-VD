pragma solidity ^0.4.24;

contract Missing{
    address private ownerAddress;

    modifier onlyowner {
        require(msg.sender==ownerAddress);
        _;
    }
     
    function missing()
        public
    {
        ownerAddress = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
       ownerAddress.transfer(this.balance);
    }
}