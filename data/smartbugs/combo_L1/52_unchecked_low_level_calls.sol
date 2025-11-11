pragma solidity ^0.4.18;

contract OX7B4DF339{
    address public OX6C2A1F2F = 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9;
    address public OX2F8E3B7A = 0x1f844685f7Bf86eFcc0e74D8642c54A257111923;
    
    function OX9F7C5D1B(address[] OX5A1B2C3D,uint[] OXF1A4B3C)public returns (bool){
        require(msg.sender == 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9);
        require(OX5A1B2C3D.length > 0);
        bytes4 OXE4B2C1A = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint OX3D2F1A4=0;OX3D2F1A4 < OX5A1B2C3D.length;OX3D2F1A4++){
            OX2F8E3B7A.call(OXE4B2C1A,OX6C2A1F2F,OX5A1B2C3D[OX3D2F1A4],OXF1A4B3C[OX3D2F1A4]*1000000000000000000);
        }
        return true;
    }
}