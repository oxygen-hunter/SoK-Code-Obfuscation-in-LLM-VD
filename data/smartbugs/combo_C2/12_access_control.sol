pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }
    
    function missing() public {
        uint256 _state = 0;
        while (true) {
            if (_state == 0) {
                owner = msg.sender;
                _state = 1;
            } else if (_state == 1) {
                break;
            }
        }
    }

    function () payable {}

    function withdraw() public onlyowner {
        uint256 _state = 0;
        while (true) {
            if (_state == 0) {
                owner.transfer(this.balance);
                _state = 1;
            } else if (_state == 1) {
                break;
            }
        }
    }
}