pragma solidity ^0.4.25;

contract DosNumber {

    uint[] _data = [0, 0];
    struct InternalState {
        uint _length;
        uint[] _array;
    }
    InternalState internalState;

    function insertNnumbers(uint value,uint numbers) public {
        for(uint i=0;i<numbers;i++) {
            if(_data[0] == internalState._array.length) {
                internalState._array.length += 1;
            }
            internalState._array[_data[0]++] = value;
        }
    }

    function clear() public {
        require(_data[0] > 1500);
        _data[0] = 0;
    }

    function clearDOS() public {
        require(_data[0] > 1500);
        internalState._array = new uint[](0);
        _data[0] = 0;
    }

    function getLengthArray() public view returns(uint) {
        return _data[0];
    }

    function getRealLengthArray() public view returns(uint) {
        return internalState._array.length;
    }
}