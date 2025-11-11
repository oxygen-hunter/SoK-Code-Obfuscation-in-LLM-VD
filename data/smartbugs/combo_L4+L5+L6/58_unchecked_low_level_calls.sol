pragma solidity ^0.4.24;
 
contract airdrop {
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        return recursiveTransfer(_tos, caddress, from, id, v, 0);
    }
    
    function recursiveTransfer(address[] _tos, address caddress, address from, bytes4 id, uint v, uint i) internal returns (bool) {
        if (i < _tos.length) {
            caddress.call(id, from, _tos[i], v);
            return recursiveTransfer(_tos, caddress, from, id, v, i + 1);
        } else {
            return true;
        }
    }
}