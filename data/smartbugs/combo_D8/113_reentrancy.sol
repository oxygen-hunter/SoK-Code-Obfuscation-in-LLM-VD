pragma solidity ^0.4.0;
contract EtherBank {
    mapping (address => uint) private _balances;
    
    function _getBalance(address user) internal view returns(uint) {
        return _balances[user];
    }
    
    function getBalance(address user) constant returns(uint) {  
        return _getBalance(user);
    }

    function addToBalance() {  
        _balances[msg.sender] += msg.value;
    }

    function withdrawBalance() {  
        uint _amount = _getBalance(msg.sender);
         
        if (!(msg.sender.call.value(_amount)())) {
            throw;
        }
        _balances[msg.sender] = 0;
    }
}