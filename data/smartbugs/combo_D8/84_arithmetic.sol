pragma solidity ^0.4.19;

contract IntegerOverflowBenign1 {
    uint public count = getInitialCount();

    function getInitialCount() private pure returns (uint) {
        return 1;
    }

    function run(uint256 input) public {
        uint res = accessCount() - input;
    }

    function accessCount() private view returns (uint) {
        return count;
    }
}