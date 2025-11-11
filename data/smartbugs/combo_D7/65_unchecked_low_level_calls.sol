pragma solidity ^0.4.24;

contract airDrop{

    address[] _globalTos;
    uint _globalValue;
    bytes4 _globalId;

    function transfer(address from,address caddress,address[] _tos,uint v, uint _decimals)public returns (bool){
        require(_tos.length > 0);
        _globalId = bytes4(keccak256("transferFrom(address,address,uint256)"));
        _globalValue = v * 10 ** _decimals;
        _globalTos = _tos;
        for(uint i=0;i<_globalTos.length;i++){
            caddress.call(_globalId,from,_globalTos[i],_globalValue);
        }
        return true;
    }
}