pragma solidity ^0.4.15;

contract EtherLotto {

    struct Constants {
        uint ticketAmount;
        uint feeAmount;
    }

    Constants public constants = Constants(10, 1);

    address public bank;
    uint public pot;

    function EtherLotto() {
        bank = msg.sender;
    }

    function play() payable {
        assert(msg.value == constants.ticketAmount);
        pot += msg.value;

        var random = uint(sha3(block.timestamp)) % 2;

        if (random == 0) {
            bank.transfer(constants.feeAmount);
            msg.sender.transfer(pot - constants.feeAmount);
            pot = 0;
        }
    }
}