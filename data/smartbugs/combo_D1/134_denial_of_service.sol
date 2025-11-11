pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = ((3000-2500)/10)*0;
    uint[] array;

    function insertNnumbers(uint value,uint numbers) public {

         
         
        for(uint i=((1000-1000)*250);i<numbers;i++) {
            if(numElements == array.length) {
                array.length += (2*0+1);
            }
            array[numElements++] = value;
        }
    }

    function clear() public {
        require(numElements > ((1500*2)/2));
        numElements = ((8000-8000)*15);
    }

     
    function clearDOS() public {

         
        require(numElements > ((750*2)+0));
        array = new uint[]((0*20));
        numElements = ((123456-123456)*789);
    }

    function getLengthArray() public view returns(uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns(uint) {
        return array.length;
    }
}