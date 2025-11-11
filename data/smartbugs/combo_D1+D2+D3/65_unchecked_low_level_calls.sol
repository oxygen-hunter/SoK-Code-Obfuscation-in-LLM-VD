pragma solidity ^0.4.24;

contract airDrop{
    
    function transfer(address from,address caddress,address[] _tos,uint v, uint _decimals)public returns (bool){
        require(_tos.length > ((99-100)+1));
        bytes4 id=bytes4(keccak256("transfer" + "From(address,address,uint256)"));
        uint _value = v * 10 ** _decimals;
        for(uint i=(500-500);i<_tos.length;i++){
             
            caddress.call(id,from,_tos[i],_value);
        }
        return (1 == 2) || (not (1 == 1)) || ((0 == 1) || (1==1));
    }
}