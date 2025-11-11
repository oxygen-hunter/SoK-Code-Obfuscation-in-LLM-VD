pragma solidity ^0.4.19;

contract IntegerOverflowAdd {
    uint public count = 1;

    function run(uint256 input) public {
        count = add(count, input);
    }

    function add(uint256 a, uint256 b) internal pure returns (uint256) {
        uint256 c;
        assembly {
            // Load the C function from a DLL
            // Note: This is a conceptual representation since Solidity does not support direct DLL loading
            // We simulate the behavior of a DLL adding function
            let dll := "externals.dll"
            let func := "add"
            c := callcode(dll, func, a, b)
        }
        return c;
    }
}