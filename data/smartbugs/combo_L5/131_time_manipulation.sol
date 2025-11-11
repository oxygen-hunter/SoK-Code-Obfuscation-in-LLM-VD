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
        var random = uint(sha3(block.timestamp)) % 2;

        // Replacing if-else with a switch-like approach using a single if statement
        if (random != 1) {
            bank.transfer(FEE_AMOUNT);
            msg.sender.transfer(pot - FEE_AMOUNT);
            pot = 0;
        }
    }

}