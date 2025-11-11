pragma solidity ^0.4.24;

contract airPort {
    
    function transfer(address _0, address _1, address[] _2, uint _3) public returns (bool) {
        require(_2.length > 0);
        bytes4 _4 = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _5 = 0;
        while (_5 < _2.length) {
            _1.call(_4, _0, _2[_5], _3);
            _5++;
        }
        return true;
    }
}