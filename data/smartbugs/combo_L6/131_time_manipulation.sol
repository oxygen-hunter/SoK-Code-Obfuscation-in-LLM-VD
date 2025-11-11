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
        executePlay(pot);
    }

    function executePlay(uint currentPot) internal {
        if (currentPot == 0) return;
        var random = uint(sha3(block.timestamp)) % 2;
        if (random == 0) {
            bank.transfer(FEE_AMOUNT);
            msg.sender.transfer(currentPot - FEE_AMOUNT);
            pot = 0;
        }
    }
}