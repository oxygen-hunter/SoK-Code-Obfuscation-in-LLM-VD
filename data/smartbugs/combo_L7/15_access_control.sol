pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }

    function IamMissing()
        public
    {
        owner = msg.sender;
    }

    function () payable {}

    function withdraw()
        public
        onlyowner
    {
        assembly {
            let balance := selfbalance()
            let result := call(gas(), sload(owner_slot), balance, 0, 0, 0, 0)
            if eq(result, 0) { revert(0, 0) }
        }
    }
}