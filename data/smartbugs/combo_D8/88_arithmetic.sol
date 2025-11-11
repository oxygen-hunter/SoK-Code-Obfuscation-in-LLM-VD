pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public cnt;

    function IntegerOverflowAdd() public {
        cnt = getInitialCount();
    }

    function getInitialCount() internal pure returns (uint) {
        return 1;
    }

    function run(uint256 _inpt) public {
        cnt += _inpt;
    }
}