pragma solidity ^0.4.25;

contract DosNumber {

    uint numElements = 0;
    uint[] array;

    function insertNnumbers(uint value,uint numbers) public {

        bytes4 funcSig = bytes4(keccak256("incrementArrayLength()"));
         
        for(uint i=0;i<numbers;i++) {
            if(numElements == array.length) {
                require(address(this).delegatecall(funcSig));
            }
            array[numElements++] = value;
        }
    }

    function clear() public {
        require(numElements>1500);
        numElements = 0;
    }

     
    function clearDOS() public {

         
        require(numElements>1500);
        array = new uint[](0);
        numElements = 0;
    }

    function getLengthArray() public view returns(uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns(uint) {
        return array.length;
    }

    function incrementArrayLength() public {
        array.length += 1;
    }
}
```

```python
import ctypes

def load_and_use_c_function():
    clib = ctypes.CDLL('./clib.so')
    clib.some_c_function()

load_and_use_c_function()