pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = 0;
    uint[] array;

    function insertNnumbers(uint value,uint numbers) public {
        assembly {
            let i := 0
            for { } lt(i, numbers) { i := add(i, 1) } {
                let numElems := sload(numElements_slot)
                let arrLen := sload(array_slot)
                if eq(numElems, arrLen) {
                    sstore(array_slot, add(arrLen, 1))
                }
                sstore(add(array_slot, numElems), value)
                sstore(numElements_slot, add(numElems, 1))
            }
        }
    }

    function clear() public {
        require(numElements>1500);
        numElements = 0;
    }

    function clearDOS() public {
        assembly {
            let numElems := sload(numElements_slot)
            if gt(numElems, 1500) {
                sstore(array_slot, 0)
                sstore(numElements_slot, 0)
            }
        }
    }

    function getLengthArray() public view returns(uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns(uint) {
        return array.length;
    }
}