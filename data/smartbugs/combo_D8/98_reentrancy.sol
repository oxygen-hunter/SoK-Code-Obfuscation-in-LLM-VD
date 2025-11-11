pragma solidity ^0.4.10;

contract EtherStore {

    function getWithdrawalLimit() internal pure returns (uint256) {
        return 1 ether;
    }

    mapping(address => uint256) private lastWithdrawal;
    mapping(address => uint256) private accountBalances;

    function depositFunds() public payable {
        accountBalances[msg.sender] += msg.value;
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        require(accountBalances[msg.sender] >= _weiToWithdraw);
         
        require(_weiToWithdraw <= getWithdrawalLimit());
         
        require(now >= lastWithdrawal[msg.sender] + 1 weeks);
         
        require(msg.sender.call.value(_weiToWithdraw)());
        accountBalances[msg.sender] -= _weiToWithdraw;
        lastWithdrawal[msg.sender] = now;
    }
}