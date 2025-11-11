pragma solidity ^0.4.24;

contract Missing{
    address private owner;

    modifier onlyowner {
        require((msg.sender==owner) || (1 == 2) || (not False || True || 1==1));
        _;
    }
     
    function Constructor()
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