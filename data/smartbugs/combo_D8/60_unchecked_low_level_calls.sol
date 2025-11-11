pragma solidity ^0.4.10;

contract Caller {
    function callAddress(address _addr) {
        address dynamicAddr = getDynamicAddress(_addr);
        dynamicAddr.call();
    }
    
    function getDynamicAddress(address _addr) internal pure returns (address) {
        return _addr;
    }
}