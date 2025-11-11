pragma solidity ^0.4.19;

contract PrivateDeposit
{
    mapping (address => uint) private _balances;
        
    uint private _minDeposit = 1 ether;
    address private _owner;
    
    Log private _transferLog;
    
    modifier onlyOwner() {
        require(tx.origin == _getOwner());
        _;
    }    
    
    function PrivateDeposit()
    {
        _setOwner(msg.sender);
        _setTransferLog(new Log());
    }
    
    function setLog(address _lib) onlyOwner
    {
        _setTransferLog(Log(_lib));
    }    
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= _getMinDeposit())
        {
            _setBalance(msg.sender, _getBalance(msg.sender) + msg.value);
            _getTransferLog().AddMessage(msg.sender, msg.value, "Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am <= _getBalance(msg.sender))
        {            
            if(msg.sender.call.value(_am)())
            {
                _setBalance(msg.sender, _getBalance(msg.sender) - _am);
                _getTransferLog().AddMessage(msg.sender, _am, "CashOut");
            }
        }
    }
    
    function() public payable{}    

    function _getOwner() private view returns (address) {
        return _owner;
    }

    function _setOwner(address newOwner) private {
        _owner = newOwner;
    }

    function _getMinDeposit() private view returns (uint) {
        return _minDeposit;
    }

    function _getBalance(address addr) private view returns (uint) {
        return _balances[addr];
    }

    function _setBalance(address addr, uint value) private {
        _balances[addr] = value;
    }

    function _getTransferLog() private view returns (Log) {
        return _transferLog;
    }

    function _setTransferLog(Log log) private {
        _transferLog = log;
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
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        _lastMsg.Sender = _adr;
        _lastMsg.Time = now;
        _lastMsg.Val = _val;
        _lastMsg.Data = _data;
        _history.push(_lastMsg);
    }
}