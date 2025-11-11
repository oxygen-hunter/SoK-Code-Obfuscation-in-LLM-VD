pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) public balances;

    function() payable public {
        balances[msg.sender] += msg.value;
    }

    function withdraw() public {
        assembly {
            let amount := sload(add(balances_slot, caller))
            if iszero(call(gas, caller, amount, 0, 0, 0, 0)) {
                revert(0, 0)
            }
            sstore(add(balances_slot, caller), 0)
        }
    }
}