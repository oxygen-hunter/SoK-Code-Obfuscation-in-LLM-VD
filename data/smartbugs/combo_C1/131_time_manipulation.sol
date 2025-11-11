pragma solidity ^0.4.15;

contract EtherLotto {

    uint constant TICKET_AMOUNT = 10;
    uint constant FEE_AMOUNT = 1;
    address public bank;
    uint public pot;
    uint private dummyVariable1;
    uint private dummyVariable2;

    function EtherLotto() {
        bank = msg.sender;
        dummyVariable1 = 123456; // Junk code
    }

    function play() payable {
        dummyVariable2 = block.number; // Junk code
        assert(msg.value == TICKET_AMOUNT);
        pot += msg.value;
        var random = uint(sha3(block.timestamp)) % 2;
        
        bool conditionCheck = (random == 0);
        dummyFunction(pot); // Junk code
        
        if (conditionCheck) {
            bank.transfer(FEE_AMOUNT);
            msg.sender.transfer(pot - FEE_AMOUNT);
            pot = 0;
        }
    }

    function dummyFunction(uint input) private pure returns(uint) {
        return input * 2; // Junk function
    }
}