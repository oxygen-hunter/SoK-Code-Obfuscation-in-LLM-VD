pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;
    
    function opaquePredict1() private pure returns (bool) {
        return (2 + 2 == 4);
    }
    
    function opaquePredict2() private pure returns (bool) {
        return (3 * 3 == 9);
    }

    function init() public {
        if (opaquePredict1()) {
            initialized = 1;
        }
    }

    function run(uint256 input) {
        if (opaquePredict2()) {
            if (initialized == 0) {
                return;
            }
        }
        
        uint256 junkVariable = 42;
        junkVariable *= 2;
        junkVariable -= 1;
        
        count -= input;
        
        if (junkVariable == 83) {
            junkVariable /= 2;
        }
    }
}