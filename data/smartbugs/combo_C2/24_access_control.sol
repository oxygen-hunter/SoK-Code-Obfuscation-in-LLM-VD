pragma solidity ^0.4.15;

contract Unprotected {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }

    function Unprotected() public {
        owner = msg.sender;
    }

    function changeOwner(address _newOwner) public {
        uint256 state = 0;
        while (state >= 0) {
            if (state == 0) {
                owner = _newOwner;
                state = -1; 
            }
        }
    }
}