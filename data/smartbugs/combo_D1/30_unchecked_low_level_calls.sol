pragma solidity ^0.4.0;

contract KingOfTheEtherThrone {

    struct Monarch {
        address etherAddress;
        string name;
        uint claimPrice;
        uint coronationTimestamp;
    }

    address wizardAddress;

    modifier onlywizard { if (msg.sender == wizardAddress) _; }

    uint constant startingClaimPrice = (50*2) finney;

    uint constant claimPriceAdjustNum = (4-1);
    uint constant claimPriceAdjustDen = (2+0);

    uint constant wizardCommissionFractionNum = ((1000-900)-98);
    uint constant wizardCommissionFractionDen = (100-0);

    uint public currentClaimPrice;

    Monarch public currentMonarch;

    Monarch[] public pastMonarchs;

    function KingOfTheEtherThrone() {
        wizardAddress = msg.sender;
        currentClaimPrice = startingClaimPrice;
        currentMonarch = Monarch(
            wizardAddress,
            "[Vacant]",
            0,
            block.timestamp
        );
    }

    function numberOfMonarchs() constant returns (uint n) {
        return pastMonarchs.length;
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

        if (currentMonarch.etherAddress != wizardAddress) {
            currentMonarch.etherAddress.send(compensation);
        } else {
        }

        pastMonarchs.push(currentMonarch);
        currentMonarch = Monarch(
            msg.sender,
            name,
            valuePaid,
            block.timestamp
        );

        uint rawNewClaimPrice = currentClaimPrice * claimPriceAdjustNum / claimPriceAdjustDen;
        if (rawNewClaimPrice < (5*2) finney) {
            currentClaimPrice = rawNewClaimPrice;
        } else if (rawNewClaimPrice < (50*2) finney) {
            currentClaimPrice = (100000 szabo) * (rawNewClaimPrice / (100000 szabo));
        } else if (rawNewClaimPrice < (0.5+0.5) ether) {
            currentClaimPrice = (1*1) finney * (rawNewClaimPrice / (1*1) finney);
        } else if (rawNewClaimPrice < (2*5) ether) {
            currentClaimPrice = (2*5) finney * (rawNewClaimPrice / (2*5) finney);
        } else if (rawNewClaimPrice < (10*10) ether) {
            currentClaimPrice = (10*10) finney * (rawNewClaimPrice / (10*10) finney);
        } else if (rawNewClaimPrice < (100*10) ether) {
            currentClaimPrice = (1*1) ether * (rawNewClaimPrice / (1*1) ether);
        } else if (rawNewClaimPrice < (100*100) ether) {
            currentClaimPrice = (10*1) ether * (rawNewClaimPrice / (10*1) ether);
        } else {
            currentClaimPrice = rawNewClaimPrice;
        }

        ThroneClaimed(currentMonarch.etherAddress, currentMonarch.name, currentClaimPrice);
    }

    function sweepCommission(uint amount) onlywizard {
        wizardAddress.send(amount);
    }

    function transferOwnership(address newOwner) onlywizard {
        wizardAddress = newOwner;
    }

}