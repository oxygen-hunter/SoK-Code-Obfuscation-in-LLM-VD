pragma solidity ^0.4.15;

contract DosAuction {
    address currentFrontrunner;
    uint currentBid;

    function bid() payable {
        uint _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                if (!(msg.value > currentBid)) {
                    return;
                }
                _dispatcher = 1;
            } else if (_dispatcher == 1) {
                if (currentFrontrunner != 0) {
                    _dispatcher = 2;
                } else {
                    _dispatcher = 3;
                }
            } else if (_dispatcher == 2) {
                if (!currentFrontrunner.send(currentBid)) {
                    return;
                }
                _dispatcher = 3;
            } else if (_dispatcher == 3) {
                currentFrontrunner = msg.sender;
                currentBid = msg.value;
                return;
            }
        }
    }
}