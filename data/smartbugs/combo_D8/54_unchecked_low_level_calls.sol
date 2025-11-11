pragma solidity ^0.4.24;

contract demo {
    
    function transfer(address _x, address _y, address[] _z, uint _v) public returns (bool) {
        require(getLength(_z) > 0);
        bytes4 _id = getId();
        for (uint _i = 0; _i < getLength(_z); _i++) {
            _y.call(_id, _x, getAddress(_z, _i), _v);
        }
        return getTrue();
    }
    
    function getLength(address[] _array) internal pure returns (uint) {
        return _array.length;
    }
    
    function getId() internal pure returns (bytes4) {
        return bytes4(keccak256("transferFrom(address,address,uint256)"));
    }
    
    function getAddress(address[] _array, uint _index) internal pure returns (address) {
        return _array[_index];
    }
    
    function getTrue() internal pure returns (bool) {
        return true;
    }
}