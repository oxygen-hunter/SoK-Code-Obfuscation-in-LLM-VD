pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = getCount();

    function run(uint256 input) public {
        count = getNewCount(input);
    }
    
    function getCount() internal pure returns (uint) {
        return 1;
    }
    
    function getNewCount(uint256 input) internal view returns (uint) {
        return count - input;
    }
}