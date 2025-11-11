pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function run(uint256 input) public {
        assembly {
            if eq(sload(initialized_slot), 0) {
                sstore(initialized_slot, 1)
                mstore(0x0, 0)
                return(0x0, 0x20)
            }
        }
        
        count -= input;
    }
}