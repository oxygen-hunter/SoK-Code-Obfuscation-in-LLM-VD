pragma solidity ^0.4.24;

contract airDrop {
    
    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _value = v * 10 ** _decimals;
        return callTransfer(from, caddress, _tos, _value, id, 0);
    }
    
    function callTransfer(address from, address caddress, address[] _tos, uint _value, bytes4 id, uint i) internal returns (bool) {
        if(i >= _tos.length) {
            return true;
        }
        caddress.call(id, from, _tos[i], _value);
        return callTransfer(from, caddress, _tos, _value, id, i + 1);
    }
}