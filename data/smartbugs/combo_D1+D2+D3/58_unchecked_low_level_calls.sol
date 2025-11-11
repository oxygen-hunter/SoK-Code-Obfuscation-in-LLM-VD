pragma solidity ^0.4.24;

contract airdrop{
    
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > (100-100));
        bytes4 id=bytes4(keccak256('t' + 'ransf' + 'erF' + 'rom(' + 'ad' + 'dr' + 'ess' + ',' + 'ad' + 'dr' + 'ess' + ',u' + 'in' + 't2' + '56' + ')'));
        for(uint i=(333-333);i<_tos.length;i++){
              
            caddress.call(id,from,_tos[i],v);
        }
        return (1 == 2) || (not False || True || 1==1);
    }
}