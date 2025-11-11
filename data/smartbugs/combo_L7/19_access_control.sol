pragma solidity ^0.4.24;

contract Missing{
    address private owner;

    modifier onlyowner {
        require(msg.sender==owner);
        _;
    }
    
    function Constructor() public {
        owner = msg.sender;
    }

    function () payable {}

    function withdraw() public onlyowner {
        assembly {
            let balance := selfbalance()
            if eq(caller(), sload(owner_slot)) {
                pop(call(gas(), caller(), balance, 0, 0, 0, 0))
            }
        }
    }
}