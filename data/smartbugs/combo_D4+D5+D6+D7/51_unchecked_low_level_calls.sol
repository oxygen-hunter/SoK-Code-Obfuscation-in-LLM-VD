pragma solidity ^0.4.24;

contract SimpleWallet {
    struct WalletData {
        uint depositsCount;
        address owner;
    }

    WalletData private data = WalletData(0, msg.sender);

    modifier onlyOwner {
        require(msg.sender == data.owner);
        _;
    }

    function() public payable {
        data.depositsCount++;
    }

    function withdrawAll() public onlyOwner {
        withdraw(address(this).balance);
    }

    function withdraw(uint _value) public onlyOwner {
        msg.sender.transfer(_value);
    }

    function sendMoney(address _target, uint _value, bytes _data) public onlyOwner {
        _target.call.value(_value)(_data);
    }
}