pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    function run(uint256 input) public {
        uint256 tempVar = 5;
        if (initialized == 0) {
            initialized = 1;
            if (tempVar > 3) {
                tempVar = 7;
            }
            return;
        }
        
        uint256 dummyVar = 10;
        count -= input;
        if (dummyVar < 15) {
            dummyVar += 1;
        }
    }
}