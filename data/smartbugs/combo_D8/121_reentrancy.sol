pragma solidity ^0.4.19;

contract PRIVATE_ETH_CELL
{
    mapping (address=>uint256) private _balances;   
   
    uint private _minSum;
    
    LogFile private _log;
    
    bool private _intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        require(!_getIntitalized());
        _setMinSum(_val);
    }
    
    function SetLogFile(address _log)
    public
    {
        require(!_getIntitalized());
        _setLogFile(_log);
    }
    
    function Initialized()
    public
    {
        _setIntitalized(true);
    }
    
    function Deposit()
    public
    payable
    {
        _updateBalance(msg.sender, msg.value, true);
        _getLogFile().AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(_getBalance(msg.sender) >= _getMinSum() && _getBalance(msg.sender) >= _am)
        {   
            if(msg.sender.call.value(_am)())
            {
                _updateBalance(msg.sender, _am, false);
                _getLogFile().AddMessage(msg.sender, _am, "Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
    }
    
    function _getBalance(address _addr) internal view returns (uint256) {
        return _balances[_addr];
    }
    
    function _updateBalance(address _addr, uint256 _value, bool _isDeposit) internal {
        if (_isDeposit) {
            _balances[_addr] += _value;
        } else {
            _balances[_addr] -= _value;
        }
    }
    
    function _getMinSum() internal view returns (uint) {
        return _minSum;
    }
    
    function _setMinSum(uint _val) internal {
        _minSum = _val;
    }
    
    function _getLogFile() internal view returns (LogFile) {
        return _log;
    }
    
    function _setLogFile(address _log) internal {
        _log = LogFile(_log);
    }
    
    function _getIntitalized() internal view returns (bool) {
        return _intitalized;
    }
    
    function _setIntitalized(bool _val) internal {
        _intitalized = _val;
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
    
    Message[] private _history;
    
    Message private _lastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        _setLastMsg(_adr, _val, _data);
        _history.push(_lastMsg);
    }
    
    function _setLastMsg(address _adr, uint _val, string _data) internal {
        _lastMsg.Sender = _adr;
        _lastMsg.Time = now;
        _lastMsg.Val = _val;
        _lastMsg.Data = _data;
    }
}