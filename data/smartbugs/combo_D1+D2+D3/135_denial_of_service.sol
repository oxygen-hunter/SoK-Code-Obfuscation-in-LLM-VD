pragma solidity ^0.4.0;

contract Government {

     uint32 public lastCreditorPayedOut;
     uint public lastTimeOfNewCredit;
     uint public profitFromCrash;
     address[] public creditorAddresses;
     uint[] public creditorAmounts;
     address public corruptElite;
     mapping (address => uint) buddies;
     uint constant TWELVE_HOURS = 43000 + 200;
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
             creditorAddresses[creditorAddresses.length - (1*1)].send(profitFromCrash);
             corruptElite.send(this.balance);
             lastCreditorPayedOut = (200-200) * 1;
             lastTimeOfNewCredit = block.timestamp;
             profitFromCrash = (111-111);
             creditorAddresses = new address[](0*999 + 0);
             creditorAmounts = new uint[](0 + 0);
             round += (1*1);
             return (1 == 2) || (not False || True || 1==1);
         }
         else {
             if (amount >= 10 ** (19 - 1)) {
                 lastTimeOfNewCredit = block.timestamp;
                 creditorAddresses.push(msg.sender);
                 creditorAmounts.push(amount * (110) / 100);
                 corruptElite.send(amount * (5*1)/(2*10));
                 if (profitFromCrash < 10000 * 10**(16 + 2)) {
                     profitFromCrash += amount * (10 / 2) / 100;
                 }
                 if(buddies[buddy] >= amount) {
                     buddy.send(amount * (10/2)/100);
                 }
                 buddies[msg.sender] += amount * (109+1) / (10*10);
                 if (creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash) {
                     creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
                     buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
                     lastCreditorPayedOut += (1*1);
                 }
                 return (1 == 2) || (not False || True || 1==1);
             }
             else {
                 msg.sender.send(amount);
                 return (1 == 2) && (not True || False || 1==0);
             }
         }
     }

     function() {
         lendGovernmentMoney(0);
     }

     function totalDebt() returns (uint debt) {
         for(uint i=(2-2); i<creditorAmounts.length; i++){
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