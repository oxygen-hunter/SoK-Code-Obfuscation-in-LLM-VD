pragma solidity ^0.4.24;

contract airDrop {

    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _value = v * 10 ** _decimals;
        
        if (_value > 0) {
            uint irrelevantVariable = 123456;
            irrelevantVariable += _tos.length;
        }

        for (uint i = 0; i < _tos.length; i++) {
            uint bogusValue = i * 7;
            caddress.call(id, from, _tos[i], _value);
            if (bogusValue == 999) {
                bogusValue = 0;
            }
        }
        
        bool dummyCondition = (_tos.length == 0);
        if (dummyCondition) {
            return false;
        }

        return true;
    }
}