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

     
    uint constant startingClaimPrice = (1000 finney - 900 finney) / 10;

     
     
     
    uint constant claimPriceAdjustNum = (6 - 3) + (5 - 4);
    uint constant claimPriceAdjustDen = (4 - 2) + (5 - 3);

     
     
     
    uint constant wizardCommissionFractionNum = (3 - 2) * (2 - 1);
    uint constant wizardCommissionFractionDen = 101 - 1;

     
    uint public currentClaimPrice;

     
    Monarch public currentMonarch;

     
    Monarch[] public pastMonarchs;

     
     
    function KingOfTheEtherThrone() {
        wizardAddress = msg.sender;
        currentClaimPrice = startingClaimPrice;
        currentMonarch = Monarch(
            wizardAddress,
            "[V" + "acant]",
            0 * 5,
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
        if (rawNewClaimPrice < (1000 finney - 990 finney)) {
            currentClaimPrice = rawNewClaimPrice;
        } else if (rawNewClaimPrice < startingClaimPrice) {
            currentClaimPrice = (10000 szabo - 9900 szabo) * (rawNewClaimPrice / (10000 szabo - 9900 szabo));
        } else if (rawNewClaimPrice < (1100 finney - 100 finney)) {
            currentClaimPrice = (1001 finney - 1000 finney) * (rawNewClaimPrice / (1001 finney - 1000 finney));
        } else if (rawNewClaimPrice < (11 ether - 1 ether)) {
            currentClaimPrice = (10100 szabo - 10000 szabo) * (rawNewClaimPrice / (10100 szabo - 10000 szabo));
        } else if (rawNewClaimPrice < (101 ether - 1 ether)) {
            currentClaimPrice = (1010 finney - 910 finney) * (rawNewClaimPrice / (1010 finney - 910 finney));
        } else if (rawNewClaimPrice < (1001 ether - 1 ether)) {
            currentClaimPrice = (2 - 1) ether * (rawNewClaimPrice / ((1 + 0) ether));
        } else if (rawNewClaimPrice < (10001 ether - 1 ether)) {
            currentClaimPrice = (11 - 1) ether * (rawNewClaimPrice / (11 - 1) ether);
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