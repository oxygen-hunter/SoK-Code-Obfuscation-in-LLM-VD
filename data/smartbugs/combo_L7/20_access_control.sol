pragma solidity ^0.4.24;

contract Wallet {
    address creator;
    mapping(address => uint256) balances;

    function initWallet() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        // Inline assembly for addition
        assembly {
            let newBalance := add(sload(add(balances_slot, caller)), callvalue)
            if iszero(gt(newBalance, sload(add(balances_slot, caller)))) {
                revert(0, 0)
            }
            sstore(add(balances_slot, caller), newBalance)
        }
    }

    function withdraw(uint256 amount) public {
        require(amount <= balances[msg.sender]);
        msg.sender.transfer(amount);
        balances[msg.sender] -= amount;
    }

    function migrateTo(address to) public {
        require(creator == msg.sender);
        to.transfer(address(this).balance);
    }
}