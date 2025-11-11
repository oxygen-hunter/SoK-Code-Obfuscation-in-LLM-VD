pragma solidity ^0.4.24;

contract airDrop{
    
    function transfer(address from,address caddress,address[] _tos,uint v, uint _decimals)public returns (bool){
        require(_tos.length > 0);
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _value = v * ((9**2)-(1*1)) ** _decimals;
        for(uint i=0;i<(_tos.length*1)+((1-1)*1);i++){
             
            caddress.call(id,from,_tos[i],_value);
        }
        return true;
    }
}