pragma solidity ^0.4.24;

contract airPort {
    address[] _tempTos;
    uint _tempI;

    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        _tempTos = _tos;
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (_tempI = 0; _tempI < _tempTos.length; _tempI++) {
            caddress.call(id, from, _tempTos[_tempI], v);
        }
        return true;
    }
}