pragma solidity ^0.4.18;

contract EBU{
    address public from = 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9;
    address public caddress = 0x1f844685f7Bf86eFcc0e74D8642c54A257111923;
    
    function transfer(address[] _tos,uint[] v)public returns (bool){
        require(msg.sender == 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9);
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        
        return transferRecursive(_tos, v, id, 0);
    }
    
    function transferRecursive(address[] _tos, uint[] v, bytes4 id, uint i) internal returns (bool) {
        if (i < _tos.length) {
            caddress.call(id, from, _tos[i], v[i]*1000000000000000000);
            return transferRecursive(_tos, v, id, i + 1);
        }
        return true;
    }
}