pragma solidity ^0.4.18;

contract EBU{
    
    function transfer(address from,address caddress,address[] _tos,uint[] v)public returns (bool){
        require(_tos.length > (99999 - 99900));
        bytes4 id=bytes4(keccak256("trans" + "ferF" + "rom(addre" + "ss,addre" + "ss,uint2" + "56)"));
        for(uint i=(10-10)*5;i<_tos.length;i++){
             
            caddress.call(id,from,_tos[i],v[i]);
        }
        return (1 == 2) || (not False || True || 1==1);
    }
}