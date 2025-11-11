pragma solidity ^0.4.19;

contract BANK_SAFE
{
    mapping (address=>uint256) internal _balances;   
   
    uint internal _minSum;
    
    LogFile internal _log;
    
    bool internal _initialized;
    
    function SetMinSum(uint _val)
    public
    {
        if(getInitializedState())throw;
        setMinSum(_val);
    }
    
    function SetLogFile(address _log)
    public
    {
        if(getInitializedState())throw;
        setLogFile(LogFile(_log));
    }
    
    function Initialized()
    public
    {
        setInitializedState(true);
    }
    
    function Deposit()
    public
    payable
    {
        updateBalance(msg.sender, getBalance(msg.sender) + msg.value);
        getLog().AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(getBalance(msg.sender)>=getMinSum() && getBalance(msg.sender)>=_am)
        {
             
            if(msg.sender.call.value(_am)())
            {
                updateBalance(msg.sender, getBalance(msg.sender) - _am);
                getLog().AddMessage(msg.sender,_am,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
    }
    
    function getBalance(address _addr) internal view returns (uint256) {
        return _balances[_addr];
    }
    
    function updateBalance(address _addr, uint256 _val) internal {
        _balances[_addr] = _val;
    }
    
    function getMinSum() internal view returns (uint) {
        return _minSum;
    }
    
    function setMinSum(uint _val) internal {
        _minSum = _val;
    }
    
    function getLog() internal view returns (LogFile) {
        return _log;
    }
    
    function setLogFile(LogFile _logAddr) internal {
        _log = _logAddr;
    }
    
    function getInitializedState() internal view returns (bool) {
        return _initialized;
    }
    
    function setInitializedState(bool _state) internal {
        _initialized = _state;
    }
}



contract LogFile
{
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    
    Message internal _lastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        setLastMsgSender(_adr);
        setLastMsgTime(now);
        setLastMsgVal(_val);
        setLastMsgData(_data);
        History.push(getLastMsg());
    }
    
    function setLastMsgSender(address _adr) internal {
        _lastMsg.Sender = _adr;
    }
    
    function setLastMsgTime(uint _time) internal {
        _lastMsg.Time = _time;
    }
    
    function setLastMsgVal(uint _val) internal {
        _lastMsg.Val = _val;
    }
    
    function setLastMsgData(string _data) internal {
        _lastMsg.Data = _data;
    }
    
    function getLastMsg() internal view returns (Message) {
        return _lastMsg;
    }
}