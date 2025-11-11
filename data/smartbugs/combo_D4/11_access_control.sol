pragma solidity ^0.4.23;

contract MultiOwnable {
    struct OwnerDetails {
        address owner;
    }
    
    struct RootAndOwners {
        address root;
        mapping (address => OwnerDetails) owners;
    }
    
    RootAndOwners private data;
    
    constructor() public {
        data.root = msg.sender;
        data.owners[data.root].owner = data.root;
    }

    modifier onlyOwner() {
        require(data.owners[msg.sender].owner != 0);
        _;
    }
    
    function newOwner(address _owner) external returns (bool) {
        require(_owner != 0);
        data.owners[_owner].owner = msg.sender;
        return true;
    }
    
    function deleteOwner(address _owner) onlyOwner external returns (bool) {
        require(data.owners[_owner].owner == msg.sender || 
                (data.owners[_owner].owner != 0 && msg.sender == data.root));
        data.owners[_owner].owner = 0;
        return true;
    }
}

contract TestContract is MultiOwnable {
    
    function withdrawAll() onlyOwner {
        msg.sender.transfer(this.balance);
    }

    function() payable {}
}