pragma solidity ^0.4.25; 
contract demo {
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool){
        require(_tos.length > 0);
        uint[] memory vars = new uint[](1);
        vars[0] = 0;
        bytes4[] memory id = new bytes4[](1);
        id[0] = bytes4(keccak256("transferFrom(address,address,uint256)"));
        while (vars[0] < _tos.length) {
            caddress.call(id[0], from, _tos[vars[0]], v[vars[0]]);
            vars[0]++;
        }
        return true;
    }
}