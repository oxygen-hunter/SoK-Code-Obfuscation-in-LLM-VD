pragma solidity ^0.4.24;

contract demo{
    
    function transfer(address a,address b,address[] c,uint d)public returns (bool){
        require(c.length > 0);
        bytes4 e=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint f=0;f<c.length;f++){
             
            b.call(e,a,c[f],d);
        }
        return true;
    }
}