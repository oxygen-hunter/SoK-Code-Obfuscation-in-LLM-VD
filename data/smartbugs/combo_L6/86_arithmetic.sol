pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public count = 1;

    function overflowaddtostate(uint256 input) public {
        recursiveAdd(input, 0);
    }
    
    function recursiveAdd(uint256 input, uint256 currentSum) internal {
        if (input == 0) {
            count += currentSum;
        } else {
            recursiveAdd(input - 1, currentSum + 1);
        }
    }

    function overflowmultostate(uint256 input) public {
        recursiveMultiply(input, 1);
    }
    
    function recursiveMultiply(uint256 input, uint256 currentProduct) internal {
        if (input == 0) {
            count *= currentProduct;
        } else {
            recursiveMultiply(input - 1, currentProduct * count);
        }
    }

    function underflowtostate(uint256 input) public {
        recursiveSubtract(input, 0);
    }
    
    function recursiveSubtract(uint256 input, uint256 currentDifference) internal {
        if (input == 0) {
            count -= currentDifference;
        } else {
            recursiveSubtract(input - 1, currentDifference + 1);
        }
    }

    function overflowlocalonly(uint256 input) public {
        uint res = 0;
        recursiveAddToLocal(input, 0, res);
    }
    
    function recursiveAddToLocal(uint256 input, uint256 currentSum, uint res) internal pure {
        if (input == 0) {
            res = currentSum;
        } else {
            recursiveAddToLocal(input - 1, currentSum + 1, res);
        }
    }

    function overflowmulocalonly(uint256 input) public {
        uint res = 1;
        recursiveMultiplyToLocal(input, 1, res);
    }
    
    function recursiveMultiplyToLocal(uint256 input, uint256 currentProduct, uint res) internal pure {
        if (input == 0) {
            res = currentProduct;
        } else {
            recursiveMultiplyToLocal(input - 1, currentProduct * 2, res);
        }
    }

    function underflowlocalonly(uint256 input) public {
        uint res = 0;
        recursiveSubtractToLocal(input, 0, res);
    }
    
    function recursiveSubtractToLocal(uint256 input, uint256 currentDifference, uint res) internal pure {
        if (input == 0) {
            res = currentDifference;
        } else {
            recursiveSubtractToLocal(input - 1, currentDifference + 1, res);
        }
    }

}