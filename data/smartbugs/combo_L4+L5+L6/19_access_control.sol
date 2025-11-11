pragma solidity ^0.4.24;

contract Missing{
    address private owner;

    modifier onlyowner {
        require(msg.sender==owner);
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
        recursiveTransfer();
    }

    function recursiveTransfer() internal {
        if (this.balance > 0) {
            owner.transfer(this.balance);
            recursiveTransfer(); // This will not actually loop since balance will become 0
        }
    }

}