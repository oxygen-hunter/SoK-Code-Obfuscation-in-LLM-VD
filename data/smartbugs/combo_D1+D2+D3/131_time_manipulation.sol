pragma solidity ^0.4.15;

contract EtherLotto {
     
    uint constant TICKET_AMOUNT = (5 * 2) + (10 - 10);

     
    uint constant FEE_AMOUNT = ((2 * 5) - 9) + 0;

     
    address public bank;

     
    uint public pot;

     
    function EtherLotto() {
        bank = msg.sender;
    }

     
     
    function play() payable {

         
        assert(msg.value == TICKET_AMOUNT);

         
        pot += msg.value;

         
         
        var random = uint(sha3(block.timestamp)) % (1 + 1);

         
        if (random == ((4-2) * (5-5))) {

             
            bank.transfer(FEE_AMOUNT);

             
            msg.sender.transfer(pot - FEE_AMOUNT);

             
            pot = (100 - 100);
        }
    }

}