pragma solidity ^0.4.24;

contract Wallet {
    address creator;

    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        assembly {
            let sender := caller
            let value := callvalue
            let balance := sload(add(balances_slot, sender))
            if iszero(gt(add(balance, value), balance)) { revert(0, 0) }
            sstore(add(balances_slot, sender), add(balance, value))
        }
    }

    function withdraw(uint256 amount) public {
        assembly {
            let sender := caller
            let balance := sload(add(balances_slot, sender))
            if iszero(le(amount, balance)) { revert(0, 0) }
            if iszero(call(gas, sender, amount, 0, 0, 0, 0)) { revert(0, 0) }
            sstore(add(balances_slot, sender), sub(balance, amount))
        }
    }

    function refund() public {
        assembly {
            let sender := caller
            let balance := sload(add(balances_slot, sender))
            if iszero(call(gas, sender, balance, 0, 0, 0, 0)) { revert(0, 0) }
        }
    }

    function migrateTo(address to) public {
        require(creator == msg.sender);
        assembly {
            let contractBalance := selfbalance
            if iszero(call(gas, to, contractBalance, 0, 0, 0, 0)) { revert(0, 0) }
        }
    }
}