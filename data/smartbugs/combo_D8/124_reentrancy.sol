pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) private _balances;
    
    Log private _transferLog;
    
    function PrivateBank(address _log)
    {
        _transferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= getMinDeposit())
        {
            _balances[msg.sender]+=msg.value;
            _transferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am<=_balances[msg.sender])
        {            
             
            if(msg.sender.call.value(_am)())
            {
                _balances[msg.sender]-=_am;
                _transferLog.AddMessage(msg.sender,_am,"CashOut");
            }
        }
    }
    
    function() public payable{}    

    function getMinDeposit() private pure returns(uint) {
        return 1 ether;
    }

    function getBalance(address _addr) public view returns(uint) {
        return _balances[_addr];
    }
    
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
    
    Message[] private _history;
    
    Message private _lastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        _lastMsg.Sender = _adr;
        _lastMsg.Time = now;
        _lastMsg.Val = _val;
        _lastMsg.Data = _data;
        _history.push(_lastMsg);
    }

    function getHistory() public view returns(Message[]) {
        return _history;
    }
}