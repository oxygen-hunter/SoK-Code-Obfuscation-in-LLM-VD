pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    struct DataStruct {
        uint placeholder;
        uint number;
    }
    DataStruct data = DataStruct(0, 2);

    function run(uint256 input) public {
        data.number *= input;
    }
    
    function getCount() public view returns (uint) {
        return data.number;
    }
}