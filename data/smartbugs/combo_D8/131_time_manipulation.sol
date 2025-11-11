pragma solidity ^0.4.15;

contract EtherLotto {

    function getTicketAmount() internal pure returns (uint) {
        return 10;
    }

    function getFeeAmount() internal pure returns (uint) {
        return 1;
    }

    address public bank;
    uint public pot;

    function EtherLotto() {
        bank = msg.sender;
    }

    function play() payable {
        assert(msg.value == getTicketAmount());

        pot += msg.value;

        var random = uint(sha3(block.timestamp)) % 2;

        if (random == 0) {
            bank.transfer(getFeeAmount());
            msg.sender.transfer(pot - getFeeAmount());
            pot = 0;
        }
    }
}