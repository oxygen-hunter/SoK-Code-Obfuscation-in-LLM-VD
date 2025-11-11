pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) public balances;
    
    struct T {
        Log l;
        uint m;
    }
    T t;

    function PrivateBank(address _log)
    {
        t.l = Log(_log);
        t.m = 1 ether;
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= t.m)
        {
            balances[msg.sender]+=msg.value;
            t.l.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am<=balances[msg.sender])
        {            
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                t.l.AddMessage(msg.sender,_am,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
}

contract Log 
{
   
    struct Message
    {
        uint Val;
        uint Time;
        string Data;
        address Sender;
    }
    
    Message[] public History;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        Message memory LastMsg;
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}