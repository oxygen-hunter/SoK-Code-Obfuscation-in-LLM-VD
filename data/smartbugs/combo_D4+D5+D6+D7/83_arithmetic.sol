pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    struct V {uint256 a; uint256 b;}
    V private v = V(0, 1);

    function init() public {
        v.a = 1;
    }

    function run(uint256 input) {
        if (v.a == 0) {
            return;
        }
         
        v.b -= input;
    }

    function getCount() public view returns (uint256) {
        return v.b;
    }
}