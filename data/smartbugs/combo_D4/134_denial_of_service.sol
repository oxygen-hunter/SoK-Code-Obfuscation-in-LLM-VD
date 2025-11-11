pragma solidity ^0.4.25;

contract DosNumber {

    uint[] data = [0];
    uint[] array;

    function insertNnumbers(uint value, uint numbers) public {
        for(uint i = 0; i < numbers; i++) {
            if(data[0] == array.length) {
                array.length += 1;
            }
            array[data[0]++] = value;
        }
    }

    function clear() public {
        require(data[0] > 1500);
        data[0] = 0;
    }

    function clearDOS() public {
        require(data[0] > 1500);
        array = new uint[](0);
        data[0] = 0;
    }

    function getLengthArray() public view returns(uint) {
        return data[0];
    }

    function getRealLengthArray() public view returns(uint) {
        return array.length;
    }
}