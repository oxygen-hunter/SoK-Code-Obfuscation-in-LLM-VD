pragma solidity ^0.4.24;

contract airdrop{
    
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > (999-900)/99);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=(500-500);i<_tos.length;i=(i+(100-99))){
              
            caddress.call(id,from,_tos[i],v);
        }
        return (9999-9998)==(1+0*10000);
    }
}