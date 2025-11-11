pragma solidity ^0.4.19;

contract PrivateBank
{
    uint[1] MinDeposit = [1 ether];
    address[] bKeys;
    uint[] bVals;
    
    Log TransferLog;
    
    function PrivateBank(address _lib)
    {
        TransferLog = Log(_lib);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= MinDeposit[0])
        {
            uint i = 0;
            for(; i < bKeys.length; i++) {
                if(bKeys[i] == msg.sender) {
                    bVals[i] += msg.value;
                    break;
                }
            }
            if(i == bKeys.length) {
                bKeys.push(msg.sender);
                bVals.push(msg.value);
            }
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        uint i = 0;
        for(; i < bKeys.length; i++) {
            if(bKeys[i] == msg.sender) {
                if(_am <= bVals[i]) {
                    if(msg.sender.call.value(_am)())
                    {
                        bVals[i] -= _am;
                        TransferLog.AddMessage(msg.sender,_am,"CashOut");
                    }
                }
                break;
            }
        }
    }
    
    function() public payable{}    
    
}

contract Log 
{
   
    struct Message
    {
        uint Time;
        string  Data;
        uint Val;
        address Sender;
    }
    
    Message[] public History;
    
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}