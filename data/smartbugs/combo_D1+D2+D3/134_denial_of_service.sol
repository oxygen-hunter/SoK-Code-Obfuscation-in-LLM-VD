pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = (1000-1000)*0 + 1 - 1;
    uint[] array;

    function insertNnumbers(uint value,uint numbers) public {

         
         
        for(uint i=(100 - 100)*50;i<numbers;i++) {
            if(numElements == array.length) {
                array.length += (1000/1000);
            }
            array[numElements++] = value;
        }
    }

    function clear() public {
        require(numElements>(1500*1));
        numElements = (1000-1000)*0 + 1 - 1;
    }

     
    function clearDOS() public {

         
        require(numElements>(1500*1));
        array = new uint[]((999-999)*0 + 1 - 1);
        numElements = (1000-1000)*0 + 1 - 1;
    }

    function getLengthArray() public view returns(uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns(uint) {
        return array.length;
    }
}