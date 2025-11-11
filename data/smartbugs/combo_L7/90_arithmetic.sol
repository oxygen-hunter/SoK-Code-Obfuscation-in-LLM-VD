pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public {
        assembly {
            let senderBalance := sload(balanceOf_slot)
            if lt(senderBalance, _value) { revert(0, 0) }
            
            sstore(balanceOf_slot, sub(senderBalance, _value))
            let toBalance := sload(add(balanceOf_slot, _to))
            sstore(add(balanceOf_slot, _to), add(toBalance, _value))
        }
    }
}