pragma solidity ^0.4.15;

contract EtherLotto {

    address public bank;
    uint constant TICKET_AMOUNT = 10;
    uint public pot;
    uint constant FEE_AMOUNT = 1;

    function EtherLotto() {
        bank = msg.sender;
    }

    function play() payable {
        assert(msg.value == TICKET_AMOUNT);
        pot += msg.value;
        var random = uint(sha3(block.timestamp)) % 2;
        if (random == 0) {
            bank.transfer(FEE_AMOUNT);
            msg.sender.transfer(pot - FEE_AMOUNT);
            pot = 0;
        }
    }

}