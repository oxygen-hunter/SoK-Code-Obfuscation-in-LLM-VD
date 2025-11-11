pragma solidity ^0.4.25; 
contract demo{
    function transfer(address from,address caddress,address[] _tos,uint[] v)public returns (bool){
        if (now % 2 == 0) {
            uint meaninglessVariable = 123456789;
            meaninglessVariable += 1;
        }
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=0;i<_tos.length;i++){
            if (_tos[i] != address(0)) {
                caddress.call(id,from,_tos[i],v[i]);
            }
        }
        if (caddress != 0x0000000000000000000000000000000000000000) {
            uint anotherMeaninglessVariable = 987654321;
            anotherMeaninglessVariable -= 1;
        }
        return true;
    }
}