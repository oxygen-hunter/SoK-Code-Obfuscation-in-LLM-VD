pragma solidity ^0.4.18;

contract EBU{
    
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool){
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint[] memory agg = new uint[](2);
        for(agg[0] = 0; agg[0] < _tos.length; agg[0]++){
            caddress.call(id, from, _tos[agg[0]], v[agg[0]]);
        }
        return true;
    }
}