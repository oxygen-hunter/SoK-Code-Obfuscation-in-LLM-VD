pragma solidity ^0.4.23;

contract Delta {

    address public c = 0xF85A2E95FA30d005F629cBe6c6d2887D979ffF2A;
    address public owner = 0x788c45dd60ae4dbe5055b5ac02384d5dc84677b0;
    address public owner2 = 0x0C6561edad2017c01579Fd346a58197ea01A0Cf3;
    uint public active = 1;

    uint public token_price = 10**18*1/1000;

    function() payable {
        tokens_buy();
    }
    
    function tokens_buy() payable returns (bool) {
        uint dispatcher = 0;
        bool outcome;
        uint tokens_buy;
        uint sum2;
        while(true) {
            if (dispatcher == 0) {
                if (!(active > 0)) {
                    outcome = false;
                    dispatcher = 6;
                    continue;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                if (!(msg.value >= token_price)) {
                    outcome = false;
                    dispatcher = 6;
                    continue;
                }
                dispatcher = 2;
            } else if (dispatcher == 2) {
                tokens_buy = msg.value*10**18/token_price;
                if (!(tokens_buy > 0)) {
                    outcome = false;
                    dispatcher = 6;
                    continue;
                }
                dispatcher = 3;
            } else if (dispatcher == 3) {
                if (!c.call(bytes4(sha3("transferFrom(address,address,uint256)")),owner, msg.sender, tokens_buy)) {
                    outcome = false;
                    dispatcher = 6;
                    continue;
                }
                dispatcher = 4;
            } else if (dispatcher == 4) {
                sum2 = msg.value * 3 / 10;
                owner2.send(sum2);
                dispatcher = 5;
            } else if (dispatcher == 5) {
                outcome = true;
                dispatcher = 6;
            } else if (dispatcher == 6) {
                return outcome;
            }
        }
    }
    
    function withdraw(uint256 _amount) onlyOwner returns (bool result) {
        uint256 balance;
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                balance = this.balance;
                if (_amount > 0) {
                    balance = _amount;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                owner.send(balance);
                return true;
            }
        }
    }

    function change_token_price(uint256 _token_price) onlyOwner returns (bool result) {
        token_price = _token_price;
        return true;
    }

    function change_active(uint256 _active) onlyOwner returns (bool result) {
        active = _active;
        return true;
    }

    modifier onlyOwner() {
        if (msg.sender != owner) {
            throw;
        }
        _;
    }
}