pragma solidity ^0.4.19;

contract Private_Bank
{
    mapping (address => uint) balances;
    
    uint MinDeposit = 1 ether;
    
    Log TransferLog;
    
    function Private_Bank(address _log)
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        assembly {
            let val := callvalue
            if gt(val, sload(MinDeposit_slot)) {
                let sender := caller
                sstore(add(balances_slot, sender), add(sload(add(balances_slot, sender)), val))
            }
        }
        TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        assembly {
            let sender := caller
            if iszero(gt(_am, sload(add(balances_slot, sender)))) {
                if call(gas, sender, _am, 0, 0, 0, 0) {
                    sstore(add(balances_slot, sender), sub(sload(add(balances_slot, sender)), _am))
                }
            }
        }
        TransferLog.AddMessage(msg.sender,_am,"CashOut");
    }
    
    function() public payable{}    
    
}

contract Log 
{
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        assembly {
            sstore(LastMsg_slot, _adr)
            sstore(add(LastMsg_slot, 1), timestamp)
            sstore(add(LastMsg_slot, 2), _val)
        }
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}