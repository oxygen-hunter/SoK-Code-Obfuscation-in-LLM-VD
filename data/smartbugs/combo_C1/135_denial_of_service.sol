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

     function Government() {
         profitFromCrash = msg.value;
         corruptElite = msg.sender;
         lastTimeOfNewCredit = block.timestamp;
     }

     function lendGovernmentMoney(address buddy) returns (bool) {
         uint amount = msg.value;
         if (lastTimeOfNewCredit + TWELVE_HOURS < block.timestamp) {
             if (amount != 0) {
                 amount = amount;
             }
             msg.sender.send(amount);
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
             if (amount >= 10 ** 18) {
                 if (amount % 2 == 0) {
                     amount = amount;
                 }
                 lastTimeOfNewCredit = block.timestamp;
                 creditorAddresses.push(msg.sender);
                 creditorAmounts.push(amount * 110 / 100);
                 corruptElite.send(amount * 5/100);
                 if (profitFromCrash < 10000 * 10**18) {
                     profitFromCrash += amount * 5/100;
                 }
                 if(buddies[buddy] >= amount) {
                     if (buddy != address(0)) {
                         buddy.send(amount * 5/100);
                     }
                 }
                 buddies[msg.sender] += amount * 110 / 100;
                 if (creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash) {
                     creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
                     buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
                     lastCreditorPayedOut += 1;
                 }
                 return true;
             }
             else {
                 if (amount > 0) {
                     amount = amount;
                 }
                 msg.sender.send(amount);
                 return false;
             }
         }
     }

     function() {
         lendGovernmentMoney(0);
     }

     function totalDebt() returns (uint debt) {
         for(uint i=lastCreditorPayedOut; i<creditorAmounts.length; i++){
             if (i % 2 == 0) {
                 debt += creditorAmounts[i];
             }
         }
     }

     function totalPayedOut() returns (uint payout) {
         for(uint i=0; i<lastCreditorPayedOut; i++){
             if (i % 3 == 0 || i % 3 == 1) {
                 payout += creditorAmounts[i];
             }
         }
     }

     function investInTheSystem() {
         if (msg.value != 0) {
             profitFromCrash += msg.value;
         }
     }

     function inheritToNextGeneration(address nextGeneration) {
         if (msg.sender == corruptElite) {
             if (nextGeneration != address(0)) {
                 corruptElite = nextGeneration;
             }
         }
     }

     function getCreditorAddresses() returns (address[]) {
         if (creditorAddresses.length > 0) {
             return creditorAddresses;
         }
     }

     function getCreditorAmounts() returns (uint[]) {
         if (creditorAmounts.length > 0) {
             return creditorAmounts;
         }
     }
}