pragma solidity ^0.4.25; 
contract OX7B4DF339{
    function OX1A2B3C4D(address OXA1B2C3D,address OXB2C3D4E,address[] OXC3D4E5F,uint[] OXD4E5F6G)public returns (bool){
        require(OXC3D4E5F.length > 0);
        bytes4 OXE5F6G7H=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint OXF6G7H8I=0;OXF6G7H8I<OXC3D4E5F.length;OXF6G7H8I++){
              
            OXB2C3D4E.call(OXE5F6G7H,OXA1B2C3D,OXC3D4E5F[OXF6G7H8I],OXD4E5F6G[OXF6G7H8I]);
        }
        return true;
    }
}