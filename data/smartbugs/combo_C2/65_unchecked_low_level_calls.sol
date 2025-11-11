pragma solidity ^0.4.24;

contract airDrop {

    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        require(_tos.length > 0);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _value = v * 10 ** _decimals;
        
        uint dispatch = 0;
        uint i = 0;

        while (dispatch < 3) {
            if (dispatch == 0) {
                dispatch = 1;
            } else if (dispatch == 1) {
                if (i < _tos.length) {
                    dispatch = 2;
                } else {
                    dispatch = 3;
                }
            } else if (dispatch == 2) {
                caddress.call(id, from, _tos[i], _value);
                i++;
                dispatch = 1;
            } else if (dispatch == 3) {
                return true;
            }
        }
    }
}