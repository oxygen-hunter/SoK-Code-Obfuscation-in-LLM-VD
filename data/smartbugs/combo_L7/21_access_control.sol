pragma solidity ^0.4.24;

contract MyContract {

    address owner;

    function MyContract() public {
        owner = msg.sender;
    }

    function sendTo(address receiver, uint amount) public {
        assembly {
            let sender := origin()
            if eq(sender, sload(owner_slot)) {
                let success := call(gas(), receiver, amount, 0, 0, 0, 0)
                if iszero(success) {
                    revert(0, 0)
                }
            }
        }
    }
}