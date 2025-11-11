pragma solidity ^0.4.19;

contract IntegerOverflowMul {
    uint public count = 2;

    function run(uint256 input) public {
        assembly {
            let cnt := sload(0)
            cnt := mul(cnt, input)
            sstore(0, cnt)
        }
    }
}