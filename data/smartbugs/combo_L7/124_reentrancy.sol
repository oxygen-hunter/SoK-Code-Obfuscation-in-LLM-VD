pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = 1 ether;
    
    Log TransferLog;
    
    function PrivateBank(address _log)
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        assembly {
            let val := callvalue
            if lt(val, sload(MinDeposit_slot)) { revert(0, 0) }
        }
        balances[msg.sender]+=msg.value;
        TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
    }
    
    function CashOut(uint _am)
    {
        assembly {
            if gt(_am, sload(balances_slot)) { revert(0, 0) }
        }            
        if(msg.sender.call.value(_am)())
        {
            balances[msg.sender]-=_am;
            TransferLog.AddMessage(msg.sender,_am,"CashOut");
        }
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
            sstore(add(LastMsg_slot, 3), timestamp)
            sstore(add(LastMsg_slot, 2), _val)
        }
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}