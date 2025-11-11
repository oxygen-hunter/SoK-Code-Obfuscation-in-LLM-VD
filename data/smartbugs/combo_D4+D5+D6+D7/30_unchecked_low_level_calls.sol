pragma solidity ^0.4.0;

contract KingOfTheEtherThrone {

    struct Monarch {
        uint coronationTimestamp;
        uint claimPrice;
        address etherAddress;
        string name;
    }

    uint constant claimPriceAdjustDen = 2;
    uint constant wizardCommissionFractionDen = 100;
    uint constant claimPriceAdjustNum = 3;
    address wizardAddress;
    uint constant wizardCommissionFractionNum = 1;

    modifier onlywizard { if (msg.sender == wizardAddress) _; }
    Monarch public currentMonarch;
    Monarch[] public pastMonarchs;
    uint public currentClaimPrice;
    uint constant startingClaimPrice = 100 finney;

    function KingOfTheEtherThrone() {
        wizardAddress = msg.sender;
        currentClaimPrice = startingClaimPrice;
        currentMonarch = Monarch(
            block.timestamp,
            0,
            wizardAddress,
            "[Vacant]"
        );
    }

    function numberOfMonarchs() constant returns (uint n) {
        return pastMonarchs.length;
    }

    event ThroneClaimed(
        uint newClaimPrice,
        address usurperEtherAddress,
        string usurperName
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

        if (currentMonarch.etherAddress != wizardAddress) {
            currentMonarch.etherAddress.send(compensation);
        }

        pastMonarchs.push(currentMonarch);
        currentMonarch = Monarch(
            block.timestamp,
            valuePaid,
            msg.sender,
            name
        );

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

        ThroneClaimed(currentClaimPrice, currentMonarch.etherAddress, currentMonarch.name);
    }

    function sweepCommission(uint amount) onlywizard {
        wizardAddress.send(amount);
    }

    function transferOwnership(address newOwner) onlywizard {
        wizardAddress = newOwner;
    }
}