pragma solidity ^0.4.0;

contract Government {

     uint32 public lastCreditorPayedOut;
     uint public lastTimeOfNewCredit;
     uint public profitFromCrash;
     address[] public creditorAddresses;
     uint[] public creditorAmounts;
     address public corruptElite;
     mapping (address => uint) buddies;
     uint constant TWELVE_HOURS = 43200;
     uint8 public round;
     uint private globalAmount;
     address private globalBuddy;
     uint private localAmount;

     function Government() {
         profitFromCrash = msg.value;
         corruptElite = msg.sender;
         lastTimeOfNewCredit = block.timestamp;
     }

     function lendGovernmentMoney(address buddy) returns (bool) {
         globalAmount = msg.value;
         globalBuddy = buddy;

         if (lastTimeOfNewCredit + TWELVE_HOURS < block.timestamp) {
             msg.sender.send(globalAmount);
             creditorAddresses[creditorAddresses.length - 1].send(profitFromCrash);
             corruptElite.send(this.balance);
             lastCreditorPayedOut = 0;
             lastTimeOfNewCredit = block.timestamp;
             profitFromCrash = 0;
             creditorAddresses = new address[](0);
             creditorAmounts = new uint[](0);
             round += 1;
             return false;
         }
         else {
             if (globalAmount >= 10 ** 18) {
                 lastTimeOfNewCredit = block.timestamp;
                 creditorAddresses.push(msg.sender);
                 creditorAmounts.push(globalAmount * 110 / 100);
                 corruptElite.send(globalAmount * 5/100);

                 if (profitFromCrash < 10000 * 10**18) {
                     profitFromCrash += globalAmount * 5/100;
                 }

                 if(buddies[globalBuddy] >= globalAmount) {
                     globalBuddy.send(globalAmount * 5/100);
                 }
                 buddies[msg.sender] += globalAmount * 110 / 100;

                 if (creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash) {
                     creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
                     buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
                     lastCreditorPayedOut += 1;
                 }
                 return true;
             }
             else {
                 msg.sender.send(globalAmount);
                 return false;
             }
         }
     }

     function() {
         lendGovernmentMoney(0);
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