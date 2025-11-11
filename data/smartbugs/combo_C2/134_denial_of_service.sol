pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = 0;
    uint[] array;

    function insertNnumbers(uint value, uint numbers) public {
        uint state = 0;
        uint i = 0;

        while (state < 3) {
            if (state == 0) {
                i = 0;
                state = 1;
            } else if (state == 1) {
                if (i < numbers) {
                    state = 2;
                } else {
                    state = 3;
                }
            } else if (state == 2) {
                if (numElements == array.length) {
                    array.length += 1;
                }
                array[numElements++] = value;
                i++;
                state = 1;
            }
        }
    }

    function clear() public {
        uint state = 0;
        while (state < 2) {
            if (state == 0) {
                require(numElements > 1500);
                state = 1;
            } else if (state == 1) {
                numElements = 0;
                state = 2;
            }
        }
    }

    function clearDOS() public {
        uint state = 0;
        while (state < 3) {
            if (state == 0) {
                require(numElements > 1500);
                state = 1;
            } else if (state == 1) {
                array = new uint[](0);
                state = 2;
            } else if (state == 2) {
                numElements = 0;
                state = 3;
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