pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
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
        recursiveWithdraw(this.balance);
    }

    function recursiveWithdraw(uint amount) internal {
        if (amount > 0) {
            uint toWithdraw = amount > 1 ether ? 1 ether : amount;
            owner.transfer(toWithdraw);
            recursiveWithdraw(amount - toWithdraw);
        }
    }
}