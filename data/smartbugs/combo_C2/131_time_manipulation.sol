pragma solidity ^0.4.15;

contract EtherLotto {

    uint constant TICKET_AMOUNT = 10;
    uint constant FEE_AMOUNT = 1;
    address public bank;
    uint public pot;

    function EtherLotto() {
        bank = msg.sender;
    }

    function play() payable {
        uint _state = 0;
        while (true) {
            if (_state == 0) {
                if (!(msg.value == TICKET_AMOUNT)) {
                    assert(false);
                }
                pot += msg.value;
                _state = 1;
            } else if (_state == 1) {
                var random = uint(sha3(block.timestamp)) % 2;
                if (random == 0) {
                    _state = 2;
                } else {
                    break;
                }
            } else if (_state == 2) {
                bank.transfer(FEE_AMOUNT);
                _state = 3;
            } else if (_state == 3) {
                msg.sender.transfer(pot - FEE_AMOUNT);
                pot = 0;
                break;
            }
        }
    }

}