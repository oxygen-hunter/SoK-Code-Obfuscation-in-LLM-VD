pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        assembly { if eq(caller, sload(owner_slot)) { let x := 0 } else { revert(0, 0) } }
        _;
    }
    
    function missing() public {
        owner = msg.sender;
    }

    function () payable {}

    function withdraw() public onlyowner {
        address addr = owner;
        assembly { callcode(gas, addr, selfbalance, 0, 0, 0, 0) }
    }
}