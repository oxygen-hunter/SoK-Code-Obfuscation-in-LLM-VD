pragma solidity ^0.4.23;

contract Delta {

    struct Addresses {
        address c;
        address owner;
        address owner2;
    }
    
    struct Configs {
        uint active;
        uint token_price;
    }

    Addresses public addr = Addresses(0xF85A2E95FA30d005F629cBe6c6d2887D979ffF2A, 0x788c45dd60ae4dbe5055b5ac02384d5dc84677b0, 0x0C6561edad2017c01579Fd346a58197ea01A0Cf3);
    Configs public config = Configs(1, 10**18*1/1000);

    function() payable {
        tokens_buy();
    }

    function tokens_buy() payable returns (bool) {
        require(config.active > 0);
        require(msg.value >= config.token_price);

        uint tokens_buy = msg.value * 10**18 / config.token_price;

        require(tokens_buy > 0);

        if (!addr.c.call(bytes4(sha3("transferFrom(address,address,uint256)")), addr.owner, msg.sender, tokens_buy)) {
            return false;
        }

        uint sum2 = msg.value * 3 / 10;
        addr.owner2.send(sum2);

        return true;
    }

    function withdraw(uint256 _amount) onlyOwner returns (bool result) {
        uint256 balance;
        balance = this.balance;
        if (_amount > 0) balance = _amount;

        addr.owner.send(balance);
        return true;
    }

    function change_token_price(uint256 _token_price) onlyOwner returns (bool result) {
        config.token_price = _token_price;
        return true;
    }

    function change_active(uint256 _active) onlyOwner returns (bool result) {
        config.active = _active;
        return true;
    }

    modifier onlyOwner() {
        if (msg.sender != addr.owner) {
            throw;
        }
        _;
    }
}