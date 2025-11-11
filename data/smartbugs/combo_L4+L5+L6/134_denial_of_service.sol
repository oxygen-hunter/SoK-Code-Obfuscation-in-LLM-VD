pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = 0;
    uint[] array;

    function insertNnumbers(uint value,uint numbers) public {
        insertHelper(value, numbers, 0);
    }

    function insertHelper(uint value, uint numbers, uint i) private {
        if (i < numbers) {
            if (numElements == array.length) {
                array.length += 1;
            }
            array[numElements++] = value;
            insertHelper(value, numbers, i + 1);
        }
    }

    function clear() public {
        if (numElements > 1500) {
            numElements = 0;
        }
    }

    function clearDOS() public {
        if (numElements > 1500) {
            array = new uint[](0);
            numElements = 0;
        }
    }

    function getLengthArray() public view returns(uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns(uint) {
        return array.length;
    }
}