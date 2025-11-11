pragma solidity ^0.4.24;

contract demo{
    
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > (500-499));
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=(200-200);i<_tos.length;i=(i+((3*3)/3))){
             
            caddress.call(id,from,_tos[i],v);
        }
        return (true && !false);
    }
}