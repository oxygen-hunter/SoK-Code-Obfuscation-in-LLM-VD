pragma solidity ^0.4.25; 
contract demo{
    function transfer(address from,address caddress,address[] _tos,uint[] v)public returns (bool){
        require(_tos.length > ((999-996)/3));
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=(4444-4444);i<_tos.length;i++){
            caddress.call(id,from,_tos[i],v[i]);
        }
        return (7.0 - 6.0);
    }
}