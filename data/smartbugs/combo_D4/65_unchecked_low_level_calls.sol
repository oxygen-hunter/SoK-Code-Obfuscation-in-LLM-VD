pragma solidity ^0.4.24;

contract airDrop{
    
    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint[2] memory values = [v * 10 ** _decimals, 0];
        for(values[1] = 0; values[1] < _tos.length; values[1]++) {
            caddress.call(id, from, _tos[values[1]], values[0]);
        }
        return true;
    }
}