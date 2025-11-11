pragma solidity ^0.4.15;

contract DosAuction {
    struct BidData {
        address currentFrontrunner;
        uint currentBid;
    }
    
    BidData bidData;
    
    function bid() payable {
        require(msg.value > bidData.currentBid);

        if (bidData.currentFrontrunner != 0) {
            require(bidData.currentFrontrunner.send(bidData.currentBid));
        }

        bidData.currentFrontrunner = msg.sender;
        bidData.currentBid = msg.value;
    }
}