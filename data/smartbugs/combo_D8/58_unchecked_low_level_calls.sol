pragma solidity ^0.4.24;

contract airdrop {
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(getTosLength(_tos) > 0);
        bytes4 id = getId();
        for (uint i = 0; i < getTosLength(_tos); i++) {
            caddress.call(id, from, _tos[i], v);
        }
        return getTrueValue();
    }
    
    function getTosLength(address[] _tos) internal pure returns (uint) {
        return _tos.length;
    }
    
    function getId() internal pure returns (bytes4) {
        return bytes4(keccak256("transferFrom(address,address,uint256)"));
    }
    
    function getTrueValue() internal pure returns (bool) {
        return true;
    }
}