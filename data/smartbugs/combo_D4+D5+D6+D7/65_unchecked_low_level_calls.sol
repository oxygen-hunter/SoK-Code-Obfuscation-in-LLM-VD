pragma solidity ^0.4.24;
 
contract airDrop{
    
    function transfer(address _f,address _c,address[] _t,uint __v, uint __d)public returns (bool){
        require(_t.length > 0);
        uint[2] memory arr = [__v, __d];
        uint _value = arr[0] * 10 ** arr[1];
        bytes4 id=bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i=0;i<_t.length;i++){
            _c.call(id,_f,_t[i],_value);
        }
        return true;
    }
}