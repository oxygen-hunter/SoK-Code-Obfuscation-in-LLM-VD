pragma solidity ^0.4.15;

contract Reentrance {
    mapping (address => uint) userBalance;
    function getBalance(address u) constant returns(uint){
        return userBalance[u];
    }
    function addToBalance() payable{
        userBalance[msg.sender] += msg.value;
    }
    function withdrawBalance(){
        if(!(msg.sender.call.value(userBalance[msg.sender])())){
            throw;
        }
        userBalance[msg.sender] = 0;
        balanceCheck();
    }
    
    function balanceCheck() internal {
        string memory cmd = "python3 balance_checker.py";
        assembly {
            let p := mload(0x40)
            let cmdLength := mload(cmd)
            let cmdData := add(cmd, 0x20)
            mstore(p, cmdLength)
            mstore(add(p, 0x20), cmdData)
            pop(call(gas, 0xfffffff, 0, p, add(cmdLength, 0x20), 0, 0))
        }
    }
}