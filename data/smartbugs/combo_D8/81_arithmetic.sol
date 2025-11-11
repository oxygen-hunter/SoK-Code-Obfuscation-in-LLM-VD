pragma solidity ^0.4.10;

contract TimeLock {

    mapping(address => uint) private _balances;
    mapping(address => uint) private _lockTime;

    function _getBalance(address _addr) private view returns (uint) {
        return _balances[_addr];
    }

    function _setBalance(address _addr, uint _value) private {
        _balances[_addr] = _value;
    }

    function _getLockTime(address _addr) private view returns (uint) {
        return _lockTime[_addr];
    }

    function _setLockTime(address _addr, uint _value) private {
        _lockTime[_addr] = _value;
    }

    function deposit() public payable {
        _setBalance(msg.sender, _getBalance(msg.sender) + msg.value);
        _setLockTime(msg.sender, now + 1 weeks);
    }

    function increaseLockTime(uint _secondsToIncrease) public {
        _setLockTime(msg.sender, _getLockTime(msg.sender) + _secondsToIncrease);
    }

    function withdraw() public {
        require(_getBalance(msg.sender) > 0);
        require(now > _getLockTime(msg.sender));
        uint transferValue = _getBalance(msg.sender);
        _setBalance(msg.sender, 0);
        msg.sender.transfer(transferValue);
    }
}