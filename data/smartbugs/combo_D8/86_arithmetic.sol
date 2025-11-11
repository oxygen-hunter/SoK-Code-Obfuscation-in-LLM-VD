pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint private _secretCount = 1;
    
    function getCount() internal view returns (uint) {
        return _secretCount;
    }

    function setCount(uint newCount) internal {
        _secretCount = newCount;
    }

    function overflowaddtostate(uint256 input) public {
        uint currentCount = getCount();
        setCount(currentCount + input);
    }

    function overflowmultostate(uint256 input) public {
        uint currentCount = getCount();
        setCount(currentCount * input);
    }

    function underflowtostate(uint256 input) public {
        uint currentCount = getCount();
        setCount(currentCount - input);
    }

    function overflowlocalonly(uint256 input) public {
        uint res = getCount() + input;
    }

    function overflowmulocalonly(uint256 input) public {
        uint res = getCount() * input;
    }

    function underflowlocalonly(uint256 input) public {
        uint res = getCount() - input;
    }

}