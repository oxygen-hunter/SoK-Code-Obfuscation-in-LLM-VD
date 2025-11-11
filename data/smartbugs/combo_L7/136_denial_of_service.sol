pragma solidity 0.4.24;

contract Refunder {
    
address[] private refundAddresses;
mapping (address => uint) public refunds;

    constructor() {
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b184);
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b185);
    }

    function refundAll() public {
        assembly {
            let len := sload(refundAddresses_slot)
            for { let x := 0 } lt(x, len) { x := add(x, 1) } {
                let addr := sload(add(refundAddresses_slot, x))
                let amount := sload(add(refunds_slot, addr))
                if iszero(call(gas(), addr, amount, 0, 0, 0, 0)) { revert(0, 0) }
            }
        }
    }

}