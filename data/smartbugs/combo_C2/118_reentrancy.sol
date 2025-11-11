pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) userBalance;

    function getBalance(address u) constant returns(uint){
        uint _returnValue;
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                _returnValue = userBalance[u];
                dispatch = 1;
            } else if (dispatch == 1) {
                return _returnValue;
            }
        }
    }

    function addToBalance() payable {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                userBalance[msg.sender] += msg.value;
                dispatch = 1;
            } else if (dispatch == 1) {
                return;
            }
        }
    }

    function withdrawBalance() {
        uint dispatch = 0;
        while (true) {
            if (dispatch == 0) {
                if (!msg.sender.call.value(userBalance[msg.sender])()) {
                    dispatch = 1;
                } else {
                    dispatch = 2;
                }
            } else if (dispatch == 1) {
                throw;
            } else if (dispatch == 2) {
                userBalance[msg.sender] = 0;
                return;
            }
        }
    }
}