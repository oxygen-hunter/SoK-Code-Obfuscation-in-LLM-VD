pragma solidity ^0.4.10;

contract EtherStore {

    struct UserData {
        uint256 lastWithdrawTime;
        uint256 balance;
    }
    
    uint256 public config = 1 ether;
    mapping(address => UserData) public userData;

    function depositFunds() public payable {
        userData[msg.sender].balance += msg.value;
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        require(userData[msg.sender].balance >= _weiToWithdraw);
         
        require(_weiToWithdraw <= config);
         
        require(now >= userData[msg.sender].lastWithdrawTime + 1 weeks);
         
        require(msg.sender.call.value(_weiToWithdraw)());
        userData[msg.sender].balance -= _weiToWithdraw;
        userData[msg.sender].lastWithdrawTime = now;
    }
}