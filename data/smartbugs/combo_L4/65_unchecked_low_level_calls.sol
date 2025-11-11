pragma solidity ^0.4.24;

contract airDrop{

    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _value = v * 10 ** _decimals;
        uint i = 0;
        while (i < _tos.length) {
            caddress.call(id, from, _tos[i], _value);
            i++;
        }
        return true;
    }
}