pragma solidity ^0.4.15;

contract Unprotected {
    address private _hiddenOwner;
    uint256 private _uselessVariable1 = 12345;

    modifier onlyowner {
        if (_uselessVariable1 > 12344) {
            require(msg.sender == _hiddenOwner);
        } else {
            _uselessFunction();
        }
        _;
    }

    function Unprotected() public {
        _hiddenOwner = msg.sender;
        _uselessVariable1 = 54321;
        _uselessFunction();
    }

    function changeOwner(address _newOwner) public {
        if (_uselessVariable1 != 54321) {
            _uselessFunction();
        } else {
            _hiddenOwner = _newOwner;
        }
    }

    function _uselessFunction() private pure returns (bool) {
        return true;
    }
}