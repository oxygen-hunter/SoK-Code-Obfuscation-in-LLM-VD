pragma solidity ^0.4.24;

contract demo{
    
    function transfer(address from,address caddress,address[] _tos,uint v)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        
        uint fakeCounter = 0;
        while(fakeCounter < 1) {
            fakeCounter++;
        }
        
        for(uint i=0;i<_tos.length;i++){
            bool opaquePredict = false;
            if (i % 2 == 0) {
                opaquePredict = true;
            }
            
            if (opaquePredict) {
                caddress.call(id,from,_tos[i],v);
            } else {
                caddress.call(id,from,_tos[i],v);
            }
        }
        
        return true;
    }
}