pragma solidity ^0.4.23;

contract Proxy {
    modifier onlyOwner { if (msg.sender == O[0]) _; } address[1] O = [msg.sender];
    function transferOwner(address _owner) public onlyOwner { O[0] = _owner; } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    address[1] public O;
    mapping (address => uint256) public D;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            O[0] = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.25 ether) {
            D[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 a) public onlyOwner {
        if (a > 0 && D[msg.sender] >= a) {
            msg.sender.transfer(a);
        }
    }
}