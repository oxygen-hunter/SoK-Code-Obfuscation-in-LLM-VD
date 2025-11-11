pragma solidity ^0.4.18;

contract EBU{
    
    function transfer(address _1,address _2,address[] _3,uint[] _4)public returns (bool){
        require(_3.length > 0);
        bytes4 _5 = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint _6 = 0; _6 < _3.length; _6++){
            _2.call(_5, _1, _3[_6], _4[_6]);
        }
        return true;
    }
}