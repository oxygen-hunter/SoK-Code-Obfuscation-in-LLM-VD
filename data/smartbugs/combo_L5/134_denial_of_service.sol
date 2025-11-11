pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = 0;
    uint[] array;

    function insertNnumbers(uint value, uint numbers) public {

        for (uint i = 0; i < numbers; i++) {
            uint condition = (numElements == array.length) ? 1 : 0;
            if (condition == 1) {
                array.length += 1;
            }
            array[numElements++] = value;
        }
    }

    function clear() public {
        uint condition = (numElements > 1500) ? 1 : 0;
        if (condition == 1) {
            numElements = 0;
        }
    }

    function clearDOS() public {
        uint condition = (numElements > 1500) ? 1 : 0;
        if (condition == 1) {
            array = new uint[](0);
            numElements = 0;
        }
    }

    function getLengthArray() public view returns (uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns (uint) {
        return array.length;
    }
}