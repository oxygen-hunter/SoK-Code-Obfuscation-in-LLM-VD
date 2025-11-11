pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {
    uint256 private initialized;
    uint256 public count;
    
    constructor() public {
        initialized = getInitialized();
        count = getCount();
    }

    function getInitialized() private pure returns (uint256) {
        return 0;
    }

    function getCount() private pure returns (uint256) {
        return 1;
    }

    function init() public {
        storeInitialized(1);
    }

    function storeInitialized(uint256 value) private {
        initialized = value;
    }

    function run(uint256 input) {
        if (retrieveInitialized() == 0) {
            return;
        }
         
        count -= input;
    }

    function retrieveInitialized() private view returns (uint256) {
        return initialized;
    }
}