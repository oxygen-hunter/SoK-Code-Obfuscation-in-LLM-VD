pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = 1;

    function run(uint256 input) public {
        uint256 dummyVariable = 123456; 
        if (dummyVariable > 0) {
            count += input;
            if (count < dummyVariable) {
                dummyFunction(count);
            }
        } else {
            dummyVariable = 654321; 
        }
    }
    
    function dummyFunction(uint256 dummyInput) internal pure returns (uint256) {
        return dummyInput * 2;
    }
}