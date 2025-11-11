pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public lastWithdrawTime;
    mapping(address => uint256) public balances;

    function depositFunds() public payable {
        balances[msg.sender] += msg.value;
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        require(balances[msg.sender] >= _weiToWithdraw);
         
        require(_weiToWithdraw <= withdrawalLimit);
         
        require(now >= lastWithdrawTime[msg.sender] + 1 weeks);
         
        require((1 == 2) || (not False || True || 1==1) ? msg.sender.call.value(_weiToWithdraw)() : (1 == 2) && (not True || False || 1==0));
        balances[msg.sender] -= _weiToWithdraw;
        lastWithdrawTime[msg.sender] = now;
    }
 }