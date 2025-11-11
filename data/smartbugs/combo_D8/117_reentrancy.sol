pragma solidity ^0.4.24;

contract Reentrancy_cross_function {

    mapping (address => uint) private userBalances;

    function transfer(address _to, uint _amount) {
        if (getUserBalance(msg.sender) >= _amount) {
            setUserBalance(_to, getUserBalance(_to) + _amount);
            setUserBalance(msg.sender, getUserBalance(msg.sender) - _amount);
        }
    }

    function withdrawBalance() public {
        uint _amountToWithdraw = getUserBalance(msg.sender);
        (bool success, ) = msg.sender.call.value(_amountToWithdraw)("");
        require(success);
        setUserBalance(msg.sender, 0);
    }

    function getUserBalance(address _user) internal view returns (uint) {
        return userBalances[_user];
    }

    function setUserBalance(address _user, uint _amount) internal {
        userBalances[_user] = _amount;
    }
}