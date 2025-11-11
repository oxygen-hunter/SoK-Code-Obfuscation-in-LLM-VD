pragma solidity ^0.4.0;

contract KingOfTheEtherThrone {

    address wizardAddress;

    modifier onlywizard { if (msg.sender == wizardAddress) _; }

    uint constant startingClaimPrice = 100 finney;
    uint constant claimPriceAdjustNum = 3;
    uint constant claimPriceAdjustDen = 2;
    uint constant wizardCommissionFractionNum = 1;
    uint constant wizardCommissionFractionDen = 100;

    uint public currentClaimPrice;

    address currentMonarchEtherAddress;
    string currentMonarchName;
    uint currentMonarchClaimPrice;
    uint currentMonarchCoronationTimestamp;

    struct Monarch {
        address etherAddress;
        string name;
        uint claimPrice;
        uint coronationTimestamp;
    }

    address[] pastMonarchsEtherAddress;
    string[] pastMonarchsName;
    uint[] pastMonarchsClaimPrice;
    uint[] pastMonarchsCoronationTimestamp;

    function KingOfTheEtherThrone() {
        wizardAddress = msg.sender;
        currentClaimPrice = startingClaimPrice;
        currentMonarchEtherAddress = wizardAddress;
        currentMonarchName = "[Vacant]";
        currentMonarchClaimPrice = 0;
        currentMonarchCoronationTimestamp = block.timestamp;
    }

    function numberOfMonarchs() constant returns (uint n) {
        return pastMonarchsEtherAddress.length;
    }

    event ThroneClaimed(
        address usurperEtherAddress,
        string usurperName,
        uint newClaimPrice
    );

    function() {
        claimThrone(string(msg.data));
    }

    function claimThrone(string name) {
        uint valuePaid = msg.value;

        if (valuePaid < currentClaimPrice) {
            msg.sender.send(valuePaid);
            return;
        }

        if (valuePaid > currentClaimPrice) {
            uint excessPaid = valuePaid - currentClaimPrice;
            msg.sender.send(excessPaid);
            valuePaid = valuePaid - excessPaid;
        }

        uint wizardCommission = (valuePaid * wizardCommissionFractionNum) / wizardCommissionFractionDen;
        uint compensation = valuePaid - wizardCommission;

        if (currentMonarchEtherAddress != wizardAddress) {
            currentMonarchEtherAddress.send(compensation);
        }

        pastMonarchsEtherAddress.push(currentMonarchEtherAddress);
        pastMonarchsName.push(currentMonarchName);
        pastMonarchsClaimPrice.push(currentMonarchClaimPrice);
        pastMonarchsCoronationTimestamp.push(currentMonarchCoronationTimestamp);

        currentMonarchEtherAddress = msg.sender;
        currentMonarchName = name;
        currentMonarchClaimPrice = valuePaid;
        currentMonarchCoronationTimestamp = block.timestamp;

        uint rawNewClaimPrice = currentClaimPrice * claimPriceAdjustNum / claimPriceAdjustDen;
        if (rawNewClaimPrice < 10 finney) {
            currentClaimPrice = rawNewClaimPrice;
        } else if (rawNewClaimPrice < 100 finney) {
            currentClaimPrice = 100 szabo * (rawNewClaimPrice / 100 szabo);
        } else if (rawNewClaimPrice < 1 ether) {
            currentClaimPrice = 1 finney * (rawNewClaimPrice / 1 finney);
        } else if (rawNewClaimPrice < 10 ether) {
            currentClaimPrice = 10 finney * (rawNewClaimPrice / 10 finney);
        } else if (rawNewClaimPrice < 100 ether) {
            currentClaimPrice = 100 finney * (rawNewClaimPrice / 100 finney);
        } else if (rawNewClaimPrice < 1000 ether) {
            currentClaimPrice = 1 ether * (rawNewClaimPrice / 1 ether);
        } else if (rawNewClaimPrice < 10000 ether) {
            currentClaimPrice = 10 ether * (rawNewClaimPrice / 10 ether);
        } else {
            currentClaimPrice = rawNewClaimPrice;
        }

        ThroneClaimed(currentMonarchEtherAddress, currentMonarchName, currentClaimPrice);
    }

    function sweepCommission(uint amount) onlywizard {
        wizardAddress.send(amount);
    }

    function transferOwnership(address newOwner) onlywizard {
        wizardAddress = newOwner;
    }

}