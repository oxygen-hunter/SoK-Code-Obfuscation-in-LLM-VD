pragma solidity ^0.4.0;

contract KingOfTheEtherThrone {

    struct Monarch {
        address etherAddress;
        string name;
        uint claimPrice;
        uint coronationTimestamp;
    }

    struct Constants {
        uint startingClaimPrice;
        uint[2] claimPriceAdjust;
        uint[2] wizardCommissionFraction;
    }

    struct Config {
        address wizardAddress;
        uint currentClaimPrice;
    }

    Constants private consts = Constants({
        startingClaimPrice: 100 finney,
        claimPriceAdjust: [3, 2],
        wizardCommissionFraction: [1, 100]
    });

    Config private cfg;

    modifier onlywizard { if (msg.sender == cfg.wizardAddress) _; }

    Monarch public currentMonarch;
    Monarch[] public pastMonarchs;

    function KingOfTheEtherThrone() {
        cfg.wizardAddress = msg.sender;
        cfg.currentClaimPrice = consts.startingClaimPrice;
        currentMonarch = Monarch(
            cfg.wizardAddress,
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

        if (valuePaid < cfg.currentClaimPrice) {
            msg.sender.send(valuePaid);
            return;
        }

        if (valuePaid > cfg.currentClaimPrice) {
            uint excessPaid = valuePaid - cfg.currentClaimPrice;
            msg.sender.send(excessPaid);
            valuePaid = valuePaid - excessPaid;
        }

        uint wizardCommission = (valuePaid * consts.wizardCommissionFraction[0]) / consts.wizardCommissionFraction[1];
        uint compensation = valuePaid - wizardCommission;

        if (currentMonarch.etherAddress != cfg.wizardAddress) {
            currentMonarch.etherAddress.send(compensation);
        }

        pastMonarchs.push(currentMonarch);
        currentMonarch = Monarch(
            msg.sender,
            name,
            valuePaid,
            block.timestamp
        );

        uint rawNewClaimPrice = cfg.currentClaimPrice * consts.claimPriceAdjust[0] / consts.claimPriceAdjust[1];
        if (rawNewClaimPrice < 10 finney) {
            cfg.currentClaimPrice = rawNewClaimPrice;
        } else if (rawNewClaimPrice < 100 finney) {
            cfg.currentClaimPrice = 100 szabo * (rawNewClaimPrice / 100 szabo);
        } else if (rawNewClaimPrice < 1 ether) {
            cfg.currentClaimPrice = 1 finney * (rawNewClaimPrice / 1 finney);
        } else if (rawNewClaimPrice < 10 ether) {
            cfg.currentClaimPrice = 10 finney * (rawNewClaimPrice / 10 finney);
        } else if (rawNewClaimPrice < 100 ether) {
            cfg.currentClaimPrice = 100 finney * (rawNewClaimPrice / 100 finney);
        } else if (rawNewClaimPrice < 1000 ether) {
            cfg.currentClaimPrice = 1 ether * (rawNewClaimPrice / 1 ether);
        } else if (rawNewClaimPrice < 10000 ether) {
            cfg.currentClaimPrice = 10 ether * (rawNewClaimPrice / 10 ether);
        } else {
            cfg.currentClaimPrice = rawNewClaimPrice;
        }

        ThroneClaimed(currentMonarch.etherAddress, currentMonarch.name, cfg.currentClaimPrice);
    }

    function sweepCommission(uint amount) onlywizard {
        cfg.wizardAddress.send(amount);
    }

    function transferOwnership(address newOwner) onlywizard {
        cfg.wizardAddress = newOwner;
    }
}