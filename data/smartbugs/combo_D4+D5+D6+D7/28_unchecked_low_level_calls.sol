pragma solidity ^0.4.23;

contract Delta {
    struct Data {
        address a;
        address b;
        address c;
        uint256 d;
        uint256 e;
    }

    Data private data = Data({
        a: 0xF85A2E95FA30d005F629cBe6c6d2887D979ffF2A,
        b: 0x788c45dd60ae4dbe5055b5ac02384d5dc84677b0,
        c: 0x0C6561edad2017c01579Fd346a58197ea01A0Cf3,
        d: 1,
        e: 10**18 * 1 / 1000
    });

    function() payable {
        tokens_buy();
    }

    function tokens_buy() payable returns (bool) {
        uint x = data.d;
        require(x > 0);
        require(msg.value >= data.e);

        uint t = msg.value * 10**18 / data.e;
        require(t > 0);

        if (!data.a.call(bytes4(sha3("transferFrom(address,address,uint256)")), data.b, msg.sender, t)) {
            return false;
        }

        uint y = msg.value * 3 / 10;
        data.c.send(y);

        return true;
    }

    function withdraw(uint256 _amount) onlyOwner returns (bool result) {
        uint256 z = this.balance;
        if (_amount > 0) z = _amount;
        data.b.send(z);
        return true;
    }

    function change_token_price(uint256 _token_price) onlyOwner returns (bool result) {
        data.e = _token_price;
        return true;
    }

    function change_active(uint256 _active) onlyOwner returns (bool result) {
        data.d = _active;
        return true;
    }

    modifier onlyOwner() {
        if (msg.sender != data.b) {
            throw;
        }
        _;
    }
}