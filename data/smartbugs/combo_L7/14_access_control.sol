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
            let bal := sload(add(balances_slot, sender))
            if iszero(gt(add(bal, value), bal)) { revert(0, 0) }
            sstore(add(balances_slot, sender), add(bal, value))
        }
    }

    function withdraw(uint256 amount) public {
        assembly {
            let sender := caller
            let bal := sload(add(balances_slot, sender))
            if lt(amount, bal) { revert(0, 0) }
            if iszero(call(gas, sender, amount, 0, 0, 0, 0)) { revert(0, 0) }
            sstore(add(balances_slot, sender), sub(bal, amount))
        }
    }

    function migrateTo(address to) public {
        require(creator == msg.sender);
        assembly {
            let balance := selfbalance
            if iszero(call(gas, to, balance, 0, 0, 0, 0)) { revert(0, 0) }
        }
    }
}