pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    function transfer(address _to, uint256 _value) public {
        uint256 dispatcher = 0;
        while(true) {
            if (dispatcher == 0) {
                if (!(balanceOf[msg.sender] >= _value)) {
                    break;
                }
                dispatcher = 1;
            } else if (dispatcher == 1) {
                balanceOf[msg.sender] -= _value;
                dispatcher = 2;
            } else if (dispatcher == 2) {
                balanceOf[_to] += _value;
                break;
            }
        }
    }
}