pragma solidity ^0.4.24;

contract Wallet {
    address private cr8r;
    mapping(address => uint256) private blc;

    constructor() public {
        cr8r = getSender();
    }

    function getSender() private view returns(address) {
        return msg.sender;
    }

    function getBalance(address addr) private view returns(uint256) {
        return blc[addr];
    }

    function setBalance(address addr, uint256 value) private {
        blc[addr] = value;
    }

    function deposit() public payable {
        assert(getBalance(getSender()) + msg.value > getBalance(getSender()));
        setBalance(getSender(), getBalance(getSender()) + msg.value);
    }

    function withdraw(uint256 amnt) public {
        require(amnt <= getBalance(getSender()));
        getSender().transfer(amnt);
        setBalance(getSender(), getBalance(getSender()) - amnt);
    }

    function refund() public {
        getSender().transfer(getBalance(getSender()));
    }

    function migrateTo(address t) public {
        require(cr8r == getSender());
        t.transfer(address(this).balance);
    }
}