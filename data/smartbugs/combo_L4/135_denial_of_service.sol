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

    function Government() public {
        profitFromCrash = msg.value;
        corruptElite = msg.sender;
        lastTimeOfNewCredit = block.timestamp;
    }

    function lendGovernmentMoney(address buddy) public returns (bool) {
        uint amount = msg.value;

        if (lastTimeOfNewCredit + TWELVE_HOURS < block.timestamp) {
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
            if (amount >= 10 ** 18) {
                lastTimeOfNewCredit = block.timestamp;
                creditorAddresses.push(msg.sender);
                creditorAmounts.push(amount * 110 / 100);
                corruptElite.send(amount * 5 / 100);

                if (profitFromCrash < 10000 * 10**18) {
                    profitFromCrash += amount * 5 / 100;
                }

                if (buddies[buddy] >= amount) {
                    buddy.send(amount * 5 / 100);
                }
                buddies[msg.sender] += amount * 110 / 100;

                while (lastCreditorPayedOut < creditorAmounts.length && creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash) {
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

    function() public {
        lendGovernmentMoney(0);
    }

    function totalDebt() public returns (uint debt) {
        uint i = lastCreditorPayedOut;
        while (i < creditorAmounts.length) {
            debt += creditorAmounts[i];
            i++;
        }
    }

    function totalPayedOut() public returns (uint payout) {
        uint i = 0;
        while (i < lastCreditorPayedOut) {
            payout += creditorAmounts[i];
            i++;
        }
    }

    function investInTheSystem() public {
        profitFromCrash += msg.value;
    }

    function inheritToNextGeneration(address nextGeneration) public {
        if (msg.sender == corruptElite) {
            corruptElite = nextGeneration;
        }
    }

    function getCreditorAddresses() public returns (address[]) {
        return creditorAddresses;
    }

    function getCreditorAmounts() public returns (uint[]) {
        return creditorAmounts;
    }
}