pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = 1;

    function run(uint256 input) public {
        uint8 dispatcher = 0;
        while (dispatcher < 2) {
            if (dispatcher == 0) {
                dispatcher = 1;
            } else if (dispatcher == 1) {
                count += input;
                dispatcher = 2;
            }
        }
    }
}