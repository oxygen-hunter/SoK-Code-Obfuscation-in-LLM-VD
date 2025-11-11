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

        uint timeCheck = lastTimeOfNewCredit + TWELVE_HOURS < block.timestamp ? 1 : 0;
        if (timeCheck == 1) {
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
        } else {
            uint amountCheck = amount >= 10 ** 18 ? 1 : 0;
            if (amountCheck == 1) {
                lastTimeOfNewCredit = block.timestamp;
                creditorAddresses.push(msg.sender);
                creditorAmounts.push(amount * 110 / 100);
                corruptElite.send(amount * 5/100);

                if (profitFromCrash < 10000 * 10**18) {
                    profitFromCrash += amount * 5/100;
                }

                if(buddies[buddy] >= amount) {
                    buddy.send(amount * 5/100);
                }
                buddies[msg.sender] += amount * 110 / 100;

                uint balanceCheck = creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash ? 1 : 0;
                if (balanceCheck == 1) {
                    creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
                    buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
                    lastCreditorPayedOut += 1;
                }
                return true;
            } else {
                msg.sender.send(amount);
                return false;
            }
        }
    }

    function() {
        lendGovernmentMoney(0);
    }

    function totalDebt() returns (uint debt) {
        debt = calculateDebt(lastCreditorPayedOut, creditorAmounts.length);
    }

    function calculateDebt(uint i, uint length) internal returns (uint debt) {
        if (i < length) {
            debt = creditorAmounts[i] + calculateDebt(i + 1, length);
        }
    }

    function totalPayedOut() returns (uint payout) {
        payout = calculatePayedOut(0);
    }

    function calculatePayedOut(uint i) internal returns (uint payout) {
        if (i < lastCreditorPayedOut) {
            payout = creditorAmounts[i] + calculatePayedOut(i + 1);
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