pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public count = 1;

    function randomIncrement(uint256 value) private pure returns (uint256) {
        return value + 1;
    }

    function randomDecrement(uint256 value) private pure returns (uint256) {
        return value - 1;
    }

    function overflowaddtostate(uint256 input) public {
        if (input > 0) {
            count += input;
            uint temp = randomIncrement(count);
            count = temp - 1;
        }
    }

    function overflowmultostate(uint256 input) public {
        if (input > 1) {
            count *= input;
            uint temp = randomDecrement(count);
            count = temp + 1;
        }
    }

    function underflowtostate(uint256 input) public {
        if (input < count) {
            count -= input;
            uint temp = randomIncrement(count);
            count = temp - 1;
        }
    }

    function overflowlocalonly(uint256 input) public {
        if (input != 0) {
            uint res = count + input;
            uint temp = randomDecrement(res);
            res = temp + 1;
        }
    }

    function overflowmulocalonly(uint256 input) public {
        if (input > 1) {
            uint res = count * input;
            uint temp = randomIncrement(res);
            res = temp - 1;
        }
    }

    function underflowlocalonly(uint256 input) public {
        if (input < count) {
            uint res = count - input;
            uint temp = randomDecrement(res);
            res = temp + 1;
        }
    }
}