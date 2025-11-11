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

    uint constant startingClaimPrice = 100 finney;
    uint constant claimPriceAdjustNum = 3;
    uint constant claimPriceAdjustDen = 2;
    uint constant wizardCommissionFractionNum = 1;
    uint constant wizardCommissionFractionDen = 100;

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
        uint state = 0;
        uint next = 0;

        while (true) {
            if (state == 0) {
                if (valuePaid < currentClaimPrice) {
                    msg.sender.send(valuePaid);
                    return;
                }
                next = 1;
            } else if (state == 1) {
                if (valuePaid > currentClaimPrice) {
                    uint excessPaid = valuePaid - currentClaimPrice;
                    msg.sender.send(excessPaid);
                    valuePaid = valuePaid - excessPaid;
                }
                next = 2;
            } else if (state == 2) {
                uint wizardCommission = (valuePaid * wizardCommissionFractionNum) / wizardCommissionFractionDen;
                uint compensation = valuePaid - wizardCommission;

                if (currentMonarch.etherAddress != wizardAddress) {
                    currentMonarch.etherAddress.send(compensation);
                }
                next = 3;
            } else if (state == 3) {
                pastMonarchs.push(currentMonarch);
                currentMonarch = Monarch(
                    msg.sender,
                    name,
                    valuePaid,
                    block.timestamp
                );
                next = 4;
            } else if (state == 4) {
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
                next = 5;
            } else if (state == 5) {
                ThroneClaimed(currentMonarch.etherAddress, currentMonarch.name, currentClaimPrice);
                return;
            }
            state = next;
        }
    }

    function sweepCommission(uint amount) onlywizard {
        wizardAddress.send(amount);
    }

    function transferOwnership(address newOwner) onlywizard {
        wizardAddress = newOwner;
    }
}