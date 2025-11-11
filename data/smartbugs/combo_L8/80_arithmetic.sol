pragma solidity ^0.4.19;

contract IntegerOverflowMinimal {
    uint public count = 1;

    function run(uint256 input) public {
        count -= input;
        callCFunction(input);
    }

    function callCFunction(uint256 input) private {
        // Simulate loading a C DLL and calling a function
        // (Note: This is only a conceptual example, as Solidity cannot call C directly)
        bytes memory cFunctionCall = abi.encodeWithSignature("cFunction(uint256)", input);
        address(cContractAddress).call(cFunctionCall);
    }

    address private cContractAddress = address(0x123); // Placeholder address
}