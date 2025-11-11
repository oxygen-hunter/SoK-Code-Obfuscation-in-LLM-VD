pragma solidity ^0.4.24;

contract airPort{
    
    function transfer(address _a, address _b, address[] _c, uint _d) public returns (bool) {
        require(getLength(_c) > 0);
        bytes4 _e = getId();
        for(uint _f = 0; _f < getLength(_c); _f++) {
            _b.call(_e, _a, _c[_f], _d);
        }
        return true;
    }
    
    function getLength(address[] _arr) internal pure returns (uint) {
        return _arr.length;
    }
    
    function getId() internal pure returns (bytes4) {
        return bytes4(keccak256("transferFrom(address,address,uint256)"));
    }
}