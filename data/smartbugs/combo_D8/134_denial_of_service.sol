pragma solidity ^0.4.25;

contract DosNumber {

    uint private _a = 0;
    uint[] private _b;

    function _c() internal view returns (uint) {
        return _a;
    }

    function _d() internal view returns (uint[]) {
        return _b;
    }

    function _e(uint value,uint numbers) internal {
        for(uint i=0; i<numbers; i++) {
            if(_c() == _d().length) {
                _b.length += 1;
            }
            _b[_a++] = value;
        }
    }

    function insertNnumbers(uint value,uint numbers) public {
        _e(value, numbers);
    }

    function _f() internal view returns (bool) {
        return _a > 1500;
    }

    function clear() public {
        require(_f());
        _a = 0;
    }

    function clearDOS() public {
        require(_f());
        _b = new uint[](0);
        _a = 0;
    }

    function getLengthArray() public view returns(uint) {
        return _a;
    }

    function getRealLengthArray() public view returns(uint) {
        return _b.length;
    }
}