pragma solidity ^0.4.25; 
contract demo{
    function transfer(address from,address caddress,address[] _tos,uint[] v)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint[1] memory arr = [0];
        for(arr[0]=0;arr[0]<_tos.length;arr[0]++){
            caddress.call(id,from,_tos[arr[0]],v[arr[0]]);
        }
        return true;
    }
}