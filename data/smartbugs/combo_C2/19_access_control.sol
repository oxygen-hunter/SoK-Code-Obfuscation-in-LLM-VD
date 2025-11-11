pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }

    function Constructor() public {
        owner = msg.sender;
    }

    function() payable {}

    function withdraw() public onlyowner {
        bool execute = true;
        uint256 dispatcher = 0;
        while (execute) {
            if (dispatcher == 0) {
                owner.transfer(this.balance);
                dispatcher = 1;
            } else if (dispatcher == 1) {
                execute = false;
            }
        }
    }
}