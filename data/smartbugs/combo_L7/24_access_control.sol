pragma solidity ^0.4.15;

contract Unprotected {
    address private owner;

    modifier onlyowner {
        require(msg.sender==owner);
        _;
    }

    function Unprotected()
        public
    {
        assembly {
            sstore(owner_slot, caller)
        }
    }
    
    function changeOwner(address _newOwner)
        public
    {
        assembly {
            sstore(owner_slot, _newOwner)
        }
    }
}