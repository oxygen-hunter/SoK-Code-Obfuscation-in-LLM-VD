pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address a) {
        if (isAllowed()) {
            if (isEven(block.number)) {
                performCall(a);
            }
        } else {
            executeFallback();
        }
    }
    
    function isAllowed() internal pure returns (bool) {
        return true;
    }
    
    function isEven(uint256 num) internal pure returns (bool) {
        return num % 2 == 0;
    }
    
    function performCall(address a) internal {
        a.call();
        redundantOperation();
    }
    
    function executeFallback() internal pure {
        uint256 x = 1;
        x = x + 1;
    }
    
    function redundantOperation() internal pure {
        uint256 temp = 0;
        temp = temp + 1;
    }
}