pragma solidity ^0.4.23;

contract IntegerOverflowSingleTransaction {
    uint public count = 1;

    function overflowaddtostate(uint256 input) public {
        assembly {
            let cnt := sload(count_slot)
            sstore(count_slot, add(cnt, input))
        }
    }

    function overflowmultostate(uint256 input) public {
        assembly {
            let cnt := sload(count_slot)
            sstore(count_slot, mul(cnt, input))
        }
    }

    function underflowtostate(uint256 input) public {
        assembly {
            let cnt := sload(count_slot)
            sstore(count_slot, sub(cnt, input))
        }
    }

    function overflowlocalonly(uint256 input) public {
        assembly {
            let res := add(sload(count_slot), input)
        }
    }

    function overflowmulocalonly(uint256 input) public {
        assembly {
            let res := mul(sload(count_slot), input)
        }
    }

    function underflowlocalonly(uint256 input) public {
        assembly {
            let res := sub(sload(count_slot), input)
        }
    }
}