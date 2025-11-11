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

        assert(msg.value == TICKET_AMOUNT);
        pot += msg.value;

        uint random;
        for (bool done = false; !done; ) {
            random = uint(sha3(block.timestamp)) % 2;
            done = true;
        }

        if (random == 0) {
            bank.transfer(FEE_AMOUNT);
            msg.sender.transfer(pot - FEE_AMOUNT);
            pot = 0;
        }
    }
}