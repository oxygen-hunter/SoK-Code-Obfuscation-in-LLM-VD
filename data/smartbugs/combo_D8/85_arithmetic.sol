pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private a = 1;
    uint256 public b = getValueB();

    function getValueB() private view returns (uint256) {
        return a;
    }

    function setValueA(uint256 value) private {
        a = value;
    }

    function run(uint256 input) public {
        if (a == 0) {
            setValueA(1);
            return;
        }
         
        b -= input;
    }
}