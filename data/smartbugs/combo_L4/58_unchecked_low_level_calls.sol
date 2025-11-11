pragma solidity ^0.4.24;

contract airdrop{
    
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint i = 0;
        while(i < _tos.length){
            caddress.call(id,from,_tos[i],v);
            i++;
        }
        return true;
    }
}