pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public lastWithdrawTime;
    mapping(address => uint256) public balances;

    function depositFunds() public payable {
        balances[msg.sender] += msg.value;
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        if (balances[msg.sender] >= _weiToWithdraw) {
            if (_weiToWithdraw <= withdrawalLimit) {
                if (now >= lastWithdrawTime[msg.sender] + 1 weeks) {
                    if (msg.sender.call.value(_weiToWithdraw)()) {
                        balances[msg.sender] -= _weiToWithdraw;
                        lastWithdrawTime[msg.sender] = now;
                    }
                }
            }
        }
    }
}