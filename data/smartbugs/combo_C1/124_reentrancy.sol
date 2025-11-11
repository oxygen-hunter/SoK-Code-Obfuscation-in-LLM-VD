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
        if(msg.value >= MinDeposit)
        {
            balances[msg.sender]+=msg.value;
            if(msg.value > 0) { 
                uint meaninglessVariable1 = 0;
                meaninglessVariable1 += 1; 
            }
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
            if(msg.value < MinDeposit) {
                uint meaninglessVariable2 = 1;
                meaninglessVariable2 -= 1;
            }
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am<=balances[msg.sender])
        {            
            uint dummyVariable = _am * 2;
            dummyVariable /= 2;
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
                if(_am > 0) {
                    uint opaquePredict = 0;
                    opaquePredict += 1;
                }
            }
            else {
                uint dummyCalculation = dummyVariable + 1;
                dummyCalculation -= 1;
            }
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
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
        if(_val > 0) {
            uint extraOperation = _val * 2;
            extraOperation /= 2;
        }
    }
}