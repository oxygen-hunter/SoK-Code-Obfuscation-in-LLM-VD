pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public lwt_balances; 

    function depositFunds() public payable {
        lwt_balances[msg.sender] = (lwt_balances[msg.sender] & (2**256 - 1 << 128)) | (lwt_balances[msg.sender] + msg.value);
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        require((lwt_balances[msg.sender] & ((2**256 - 1) >> 128)) >= _weiToWithdraw);
         
        require(_weiToWithdraw <= withdrawalLimit);
         
        require(now >= ((lwt_balances[msg.sender] >> 128) & (2**256 - 1)) + 1 weeks);
         
        require(msg.sender.call.value(_weiToWithdraw)());
        lwt_balances[msg.sender] = ((lwt_balances[msg.sender] & ((2**256 - 1) >> 128)) - _weiToWithdraw) | (now << 128);
    }
 }