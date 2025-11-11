pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = 0;
    uint array_length = 0;

    function insertNnumbers(uint value,uint numbers) public {

        for(uint i=0;i<numbers;i++) {
            if(numElements == array_length) {
                array_length += 1;
            }
            numElements++;
        }
    }

    function clear() public {
        require(numElements>1500);
        numElements = 0;
    }

    function clearDOS() public {

        require(numElements>1500);
        array_length = 0;
        numElements = 0;
    }

    function getLengthArray() public view returns(uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns(uint) {
        return array_length;
    }
}