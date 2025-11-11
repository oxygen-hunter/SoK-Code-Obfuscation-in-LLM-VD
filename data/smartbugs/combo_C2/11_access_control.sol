pragma solidity ^0.4.23;

contract MultiOwnable {
    address public root;
    mapping (address => address) public owners;

    constructor() public {
        root = msg.sender;
        owners[root] = root;
    }

    modifier onlyOwner() {
        require(owners[msg.sender] != 0);
        _;
    }

    function newOwner(address _owner) external returns (bool) {
        uint256 state = 0;
        while (state < 2) {
            if (state == 0) {
                require(_owner != 0);
                state = 1;
            } else if (state == 1) {
                owners[_owner] = msg.sender;
                return true;
            }
        }
    }

    function deleteOwner(address _owner) onlyOwner external returns (bool) {
        uint256 state = 0;
        while (state < 2) {
            if (state == 0) {
                require(owners[_owner] == msg.sender || (owners[_owner] != 0 && msg.sender == root));
                state = 1;
            } else if (state == 1) {
                owners[_owner] = 0;
                return true;
            }
        }
    }
}

contract TestContract is MultiOwnable {

    function withdrawAll() onlyOwner {
        uint256 state = 0;
        while (state < 1) {
            if (state == 0) {
                msg.sender.transfer(this.balance);
                state = 1;
            }
        }
    }

    function() payable {
    }
}