pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance=0;

    function add(uint value) returns (bool){
        return addRecursive(value);
    }

    function addRecursive(uint value) internal returns (bool) {
        if (value == 0) {
            return true;
        }
        sellerBalance++;
        return addRecursive(value - 1);
    }
}