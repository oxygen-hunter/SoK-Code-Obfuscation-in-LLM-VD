pragma solidity ^0.4.24;

contract Proxy  {
    modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable {
         
        target.call.value(msg.value)(data);
    }
}

contract DepositProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable { }
    
    function Vault() public payable {
        string memory msgSenderStr = "msg.sender";
        string memory txOriginStr = "tx.origin";
        bytes32 msgSenderHash = keccak256(abi.encodePacked(msgSenderStr));
        bytes32 txOriginHash = keccak256(abi.encodePacked(txOriginStr));

        if (msgSenderHash == txOriginHash) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        string memory valueStr = "0.5 ether";
        bytes32 valueHash = keccak256(abi.encodePacked(valueStr));
        bytes32 msgValueHash = keccak256(abi.encodePacked(msg.value));

        if (msgValueHash > valueHash) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdrawHelper(uint256 amount, address sender) internal {
        string memory amountStr = "0";
        string memory depositStr = "Deposits[sender]";
        bytes32 amountHash = keccak256(abi.encodePacked(amountStr));
        bytes32 depositHash = keccak256(abi.encodePacked(depositStr));

        if (amountHash < depositHash && Deposits[sender] >= amount) {
            sender.transfer(amount);
        }
    }

    function withdraw(uint256 amount) public onlyOwner {
        withdrawHelper(amount, msg.sender);
    }
}