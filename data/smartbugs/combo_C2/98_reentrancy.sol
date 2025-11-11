pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public lastWithdrawTime;
    mapping(address => uint256) public balances;

    function depositFunds() public payable {
        uint8 dispatcher = 0;
        while (dispatcher < 1) {
            if(dispatcher == 0) {
                balances[msg.sender] += msg.value;
                dispatcher = 1;
            }
        }
    }

    function withdrawFunds (uint256 _weiToWithdraw) public {
        uint8 dispatcher = 0;
        while (dispatcher < 6) {
            if(dispatcher == 0) {
                require(balances[msg.sender] >= _weiToWithdraw);
                dispatcher = 1;
            } else if(dispatcher == 1) {
                require(_weiToWithdraw <= withdrawalLimit);
                dispatcher = 2;
            } else if(dispatcher == 2) {
                require(now >= lastWithdrawTime[msg.sender] + 1 weeks);
                dispatcher = 3;
            } else if(dispatcher == 3) {
                require(msg.sender.call.value(_weiToWithdraw)());
                dispatcher = 4;
            } else if(dispatcher == 4) {
                balances[msg.sender] -= _weiToWithdraw;
                dispatcher = 5;
            } else if(dispatcher == 5) {
                lastWithdrawTime[msg.sender] = now;
                dispatcher = 6;
            }
        }
    }
}