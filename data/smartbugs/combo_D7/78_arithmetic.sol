pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint private _hidden_count = 2;

    function run(uint256 _hidden_input) public {
         
        _hidden_count *= _hidden_input;
    }

    function getCount() public view returns (uint) {
        return _hidden_count;
    }
}