pragma solidity ^0.4.18;

contract EBU {
    
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool) {
        require(getLength(_tos) > 0);
        bytes4 id = getBytes4();
        for (uint i = 0; i < getLength(_tos); i++) {
            caddress.call(id, from, _tos[i], v[i]);
        }
        return getTrue();
    }
    
    function getLength(address[] array) internal pure returns (uint) {
        return array.length;
    }
    
    function getBytes4() internal pure returns (bytes4) {
        return bytes4(keccak256("transferFrom(address,address,uint256)"));
    }
    
    function getTrue() internal pure returns (bool) {
        return true;
    }
}