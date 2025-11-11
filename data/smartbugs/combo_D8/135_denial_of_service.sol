pragma solidity ^0.4.0;

contract Government {

    uint32 public lastCreditorPayedOut;
    uint public lastTimeOfNewCredit;
    uint public profitFromCrash;
    address[] public creditorAddresses;
    uint[] public creditorAmounts;
    address public corruptElite;
    mapping (address => uint) buddies;
    uint8 public round;

    function Government() {
        profitFromCrash = getInitialProfit();
        corruptElite = getInitialCorruptElite();
        lastTimeOfNewCredit = getInitialTime();
    }

    function getInitialProfit() internal view returns (uint) {
        return msg.value;
    }

    function getInitialCorruptElite() internal view returns (address) {
        return msg.sender;
    }

    function getInitialTime() internal view returns (uint) {
        return block.timestamp;
    }

    function TWELVE_HOURS() internal pure returns (uint) {
        return 43200;
    }

    function lendGovernmentMoney(address buddy) returns (bool) {
        uint amount = getAmount();
        if (getLastTimeOfNewCredit() + TWELVE_HOURS() < getCurrentTime()) {
            refundSender(amount);
            payOutLastCreditor();
            payOutCorruptElite();
            resetState();
            return false;
        } else {
            if (isAmountSufficient(amount)) {
                updateLastTimeOfNewCredit();
                addCreditor(msg.sender, amount);
                payCorruptElite(amount);
                updateProfitFromCrash(amount);
                payBuddy(buddy, amount);
                updateBuddyBalance(msg.sender, amount);
                payOutCreditors();
                return true;
            } else {
                refundSender(amount);
                return false;
            }
        }
    }

    function getAmount() internal view returns (uint) {
        return msg.value;
    }

    function getLastTimeOfNewCredit() internal view returns (uint) {
        return lastTimeOfNewCredit;
    }

    function getCurrentTime() internal view returns (uint) {
        return block.timestamp;
    }

    function refundSender(uint amount) internal {
        msg.sender.send(amount);
    }

    function payOutLastCreditor() internal {
        creditorAddresses[getLastCreditorIndex()].send(profitFromCrash);
    }

    function getLastCreditorIndex() internal view returns (uint) {
        return creditorAddresses.length - 1;
    }

    function payOutCorruptElite() internal {
        corruptElite.send(this.balance);
    }

    function resetState() internal {
        lastCreditorPayedOut = 0;
        lastTimeOfNewCredit = block.timestamp;
        profitFromCrash = 0;
        creditorAddresses = new address[](0);
        creditorAmounts = new uint[](0);
        incrementRound();
    }

    function isAmountSufficient(uint amount) internal pure returns (bool) {
        return amount >= 10 ** 18;
    }

    function updateLastTimeOfNewCredit() internal {
        lastTimeOfNewCredit = block.timestamp;
    }

    function addCreditor(address _address, uint amount) internal {
        creditorAddresses.push(_address);
        creditorAmounts.push(amount * 110 / 100);
    }

    function payCorruptElite(uint amount) internal {
        corruptElite.send(amount * 5/100);
    }

    function updateProfitFromCrash(uint amount) internal {
        if (profitFromCrash < getMaxProfitFromCrash()) {
            profitFromCrash += amount * 5/100;
        }
    }

    function getMaxProfitFromCrash() internal pure returns (uint) {
        return 10000 * 10**18;
    }

    function payBuddy(address buddy, uint amount) internal {
        if (buddies[buddy] >= amount) {
            buddy.send(amount * 5/100);
        }
    }

    function updateBuddyBalance(address _address, uint amount) internal {
        buddies[_address] += amount * 110 / 100;
    }

    function payOutCreditors() internal {
        if (creditorAmounts[lastCreditorPayedOut] <= getAvailableBalance()) {
            creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
            buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
            lastCreditorPayedOut++;
        }
    }

    function getAvailableBalance() internal view returns (uint) {
        return address(this).balance - profitFromCrash;
    }

    function() {
        lendGovernmentMoney(0);
    }

    function totalDebt() returns (uint debt) {
        for (uint i = lastCreditorPayedOut; i < creditorAmounts.length; i++) {
            debt += creditorAmounts[i];
        }
    }

    function totalPayedOut() returns (uint payout) {
        for (uint i = 0; i < lastCreditorPayedOut; i++) {
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

    function incrementRound() internal {
        round++;
    }
}