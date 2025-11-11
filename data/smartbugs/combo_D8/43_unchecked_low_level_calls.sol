pragma solidity ^0.4.18;

contract Lotto {

    bool[] private flags = [false];
    address[] private addresses = [0x0];
    uint[] private amounts = [0];

    function getPayedOut() public view returns (bool) {
        return flags[0];
    }

    function setPayedOut(bool _value) internal {
        flags[0] = _value;
    }

    function getWinner() public view returns (address) {
        return addresses[0];
    }

    function setWinner(address _address) internal {
        addresses[0] = _address;
    }

    function getWinAmount() public view returns (uint) {
        return amounts[0];
    }

    function setWinAmount(uint _amount) internal {
        amounts[0] = _amount;
    }

    function sendToWinner() public {
        require(!getPayedOut());
        getWinner().send(getWinAmount());
        setPayedOut(true);
    }

    function withdrawLeftOver() public {
        require(getPayedOut());
        msg.sender.send(this.balance);
    }
}