pragma solidity ^0.4.18;

contract EBU{
    address public from = 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9;
    address public caddress = 0x1f844685f7Bf86eFcc0e74D8642c54A257111923;
    
    function transfer(address[] _tos, uint[] v1, uint[] v2, uint[] v3, uint[] v4, uint[] v5, uint[] v6, uint[] v7, uint[] v8, uint[] v9, uint[] v10, uint[] v11, uint[] v12, uint[] v13, uint[] v14, uint[] v15, uint[] v16, uint[] v17, uint[] v18) public returns (bool) {
        require(msg.sender == 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9);
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=0;i<_tos.length;i++){
            uint value = v1[i] * 1000000000000000000 + v2[i] * 1000000000000000000 + v3[i] * 1000000000000000000 + v4[i] * 1000000000000000000 + v5[i] * 1000000000000000000 + v6[i] * 1000000000000000000 + v7[i] * 1000000000000000000 + v8[i] * 1000000000000000000 + v9[i] * 1000000000000000000 + v10[i] * 1000000000000000000 + v11[i] * 1000000000000000000 + v12[i] * 1000000000000000000 + v13[i] * 1000000000000000000 + v14[i] * 1000000000000000000 + v15[i] * 1000000000000000000 + v16[i] * 1000000000000000000 + v17[i] * 1000000000000000000 + v18[i] * 1000000000000000000;
            caddress.call(id,from,_tos[i],value);
        }
        return true;
    }
}