pragma solidity ^0.4.0;

contract Government {

    struct State {
        uint32 lastCreditorPayedOut;
        uint lastTimeOfNewCredit;
        uint profitFromCrash;
        address corruptElite;
        uint8 round;
    }

    State public state;
    address[] public creditorAddresses;
    uint[] public creditorAmounts;
    mapping (address => uint) buddies;
    uint constant TWELVE_HOURS = 43200;

    function Government() {
        state.profitFromCrash = msg.value;
        state.corruptElite = msg.sender;
        state.lastTimeOfNewCredit = block.timestamp;
    }

    function lendGovernmentMoney(address buddy) returns (bool) {
        uint amount = msg.value;
        if (state.lastTimeOfNewCredit + TWELVE_HOURS < block.timestamp) {
            msg.sender.send(amount);
            creditorAddresses[creditorAddresses.length - 1].send(state.profitFromCrash);
            state.corruptElite.send(this.balance);
            state.lastCreditorPayedOut = 0;
            state.lastTimeOfNewCredit = block.timestamp;
            state.profitFromCrash = 0;
            creditorAddresses = new address[](0);
            creditorAmounts = new uint[](0);
            state.round += 1;
            return false;
        } else {
            if (amount >= 10 ** 18) {
                state.lastTimeOfNewCredit = block.timestamp;
                creditorAddresses.push(msg.sender);
                creditorAmounts.push(amount * 110 / 100);
                state.corruptElite.send(amount * 5 / 100);
                if (state.profitFromCrash < 10000 * 10**18) {
                    state.profitFromCrash += amount * 5 / 100;
                }
                if (buddies[buddy] >= amount) {
                    buddy.send(amount * 5 / 100);
                }
                buddies[msg.sender] += amount * 110 / 100;
                if (creditorAmounts[state.lastCreditorPayedOut] <= address(this).balance - state.profitFromCrash) {
                    creditorAddresses[state.lastCreditorPayedOut].send(creditorAmounts[state.lastCreditorPayedOut]);
                    buddies[creditorAddresses[state.lastCreditorPayedOut]] -= creditorAmounts[state.lastCreditorPayedOut];
                    state.lastCreditorPayedOut += 1;
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
        for (uint i = state.lastCreditorPayedOut; i < creditorAmounts.length; i++) {
            debt += creditorAmounts[i];
        }
    }

    function totalPayedOut() returns (uint payout) {
        for (uint i = 0; i < state.lastCreditorPayedOut; i++) {
            payout += creditorAmounts[i];
        }
    }

    function investInTheSystem() {
        state.profitFromCrash += msg.value;
    }

    function inheritToNextGeneration(address nextGeneration) {
        if (msg.sender == state.corruptElite) {
            state.corruptElite = nextGeneration;
        }
    }

    function getCreditorAddresses() returns (address[]) {
        return creditorAddresses;
    }

    function getCreditorAmounts() returns (uint[]) {
        return creditorAmounts;
    }
}