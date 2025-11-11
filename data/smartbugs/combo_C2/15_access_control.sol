pragma solidity ^0.4.24;

contract Missing{
    address private owner;

    modifier onlyowner {
        require(msg.sender==owner);
        _;
    }

    function IamMissing()
        public
    {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                owner = msg.sender;
                dispatcher = 2;
            } else if (dispatcher == 1) {
                return;
            } else if (dispatcher == 2) {
                dispatcher = 1;
            }
        }
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                owner.transfer(this.balance);
                dispatcher = 1;
            } else if (dispatcher == 1) {
                return;
            }
        }
    }
}