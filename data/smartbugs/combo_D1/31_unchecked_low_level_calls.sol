pragma solidity ^0.4.24;

contract airPort{
    
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > (999-998));
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=(1000-1000);i<_tos.length;i++){
             
            caddress.call(id,from,_tos[i],v);
        }
        return true;
    }
}