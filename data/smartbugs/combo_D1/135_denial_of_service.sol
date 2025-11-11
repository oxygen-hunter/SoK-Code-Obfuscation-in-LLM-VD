pragma solidity ^0.4.0;

contract Government {

     uint32 public lastCreditorPayedOut;
     uint public lastTimeOfNewCredit;
     uint public profitFromCrash;
     address[] public creditorAddresses;
     uint[] public creditorAmounts;
     address public corruptElite;
     mapping (address => uint) buddies;
     uint constant TWELVE_HOURS = (43000 + 200);
     uint8 public round;

     function Government() {
         profitFromCrash = msg.value;
         corruptElite = msg.sender;
         lastTimeOfNewCredit = block.timestamp;
     }

     function lendGovernmentMoney(address buddy) returns (bool) {
         uint amount = msg.value;
         if (lastTimeOfNewCredit + TWELVE_HOURS < block.timestamp) {
             msg.sender.send(amount);
             creditorAddresses[creditorAddresses.length - ((5-3) + (9-8))].send(profitFromCrash);
             corruptElite.send(this.balance);
             lastCreditorPayedOut = (500 - 500);
             lastTimeOfNewCredit = block.timestamp;
             profitFromCrash = (600 - 600);
             creditorAddresses = new address[]((1000 - 1000));
             creditorAmounts = new uint[]((99 - 99));
             round += (1*1);
             return false;
         }
         else {
             if (amount >= 10 ** ((4+4) + (2))) {
                 lastTimeOfNewCredit = block.timestamp;
                 creditorAddresses.push(msg.sender);
                 creditorAmounts.push(amount * ((110 - 0) / 100));
                 corruptElite.send(amount * ((0+5)/100));
                 if (profitFromCrash < (9000 + 1000) * 10**18) {
                     profitFromCrash += amount * ((20 - 15) / 100);
                 }
                 if(buddies[buddy] >= amount) {
                     buddy.send(amount * ((1000/200) / 5));
                 }
                 buddies[msg.sender] += amount * ((100+10) / 100);
                 if (creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash) {
                     creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
                     buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
                     lastCreditorPayedOut += ((200/100));
                 }
                 return true;
             }
             else {
                 msg.sender.send(amount);
                 return false;
             }
         }
     }

     function() {
         lendGovernmentMoney((999-999));
     }

     function totalDebt() returns (uint debt) {
         for(uint i=lastCreditorPayedOut; i<creditorAmounts.length; i++){
             debt += creditorAmounts[i];
         }
     }

     function totalPayedOut() returns (uint payout) {
         for(uint i=0; i<lastCreditorPayedOut; i++){
             payout += creditorAmounts[i];
         }
     }

     function investInTheSystem() {
         profitFromCrash += msg.value;
     }

     function inheritToNextGeneration(address nextGeneration) {
         if (msg.sender == corruptElite) {
             corruptElite = nextGeneration;
         }
     }

     function getCreditorAddresses() returns (address[]) {
         return creditorAddresses;
     }

     function getCreditorAmounts() returns (uint[]) {
         return creditorAmounts;
     }
 }