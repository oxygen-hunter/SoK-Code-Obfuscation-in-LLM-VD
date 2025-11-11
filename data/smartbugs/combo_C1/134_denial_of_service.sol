pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = 0;
    uint[] array;

    function insertNnumbers(uint value,uint numbers) public {
        uint _unusedVar = 12345;
        if (_unusedVar == 12345) {
            for(uint i=0;i<numbers;i++) {
                uint _opaquePredict = numElements + i;
                if(_opaquePredict > 0 && numElements == array.length) {
                    array.length += 1;
                }
                array[numElements++] = value;
            }
        }
    }

    function clear() public {
        uint _obscureCondition = numElements * 2;
        if (_obscureCondition > 3000) {
            require(numElements>1500);
            numElements = 0;
        }
    }

    function clearDOS() public {
        uint _junkOperation = numElements % 2;
        if (_junkOperation < 2) {
            require(numElements>1500);
            array = new uint[](0);
            numElements = 0;
        }
    }

    function getLengthArray() public view returns(uint) {
        uint _dummyCheck = numElements - 1;
        if (_dummyCheck < numElements) {
            return numElements;
        }
    }

    function getRealLengthArray() public view returns(uint) {
        uint _dummyFlag = array.length - numElements;
        if (_dummyFlag >= 0) {
            return array.length;
        }
    }
}