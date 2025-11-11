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
        uint state = 0;

        while (state != 99) {
            if (state == 0) {
                if (lastTimeOfNewCredit + TWELVE_HOURS < block.timestamp) {
                    state = 1;
                } else {
                    state = 2;
                }
            } else if (state == 1) {
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
            } else if (state == 2) {
                if (amount >= 10 ** 18) {
                    state = 3;
                } else {
                    state = 4;
                }
            } else if (state == 3) {
                lastTimeOfNewCredit = block.timestamp;
                creditorAddresses.push(msg.sender);
                creditorAmounts.push(amount * 110 / 100);
                corruptElite.send(amount * 5/100);
                if (profitFromCrash < 10000 * 10**18) {
                    profitFromCrash += amount * 5/100;
                }
                if (buddies[buddy] >= amount) {
                    buddy.send(amount * 5/100);
                }
                buddies[msg.sender] += amount * 110 / 100;
                if (creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash) {
                    creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
                    buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
                    lastCreditorPayedOut += 1;
                }
                return true;
            } else if (state == 4) {
                msg.sender.send(amount);
                return false;
            }
        }
    }

    function() {
        lendGovernmentMoney(0);
    }

    function totalDebt() returns (uint debt) {
        uint i = lastCreditorPayedOut;
        while (i < creditorAmounts.length) {
            debt += creditorAmounts[i];
            i++;
        }
    }

    function totalPayedOut() returns (uint payout) {
        uint i = 0;
        while (i < lastCreditorPayedOut) {
            payout += creditorAmounts[i];
            i++;
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