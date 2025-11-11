pragma solidity ^0.4.24;

contract Wallet {
    address _0;
    mapping(address => uint256) _1;

    function _2() public {
        _0 = msg.sender;
    }

    function _3() public payable {
        assert(_1[msg.sender] + msg.value > _1[msg.sender]);
        _1[msg.sender] += msg.value;
    }

    function _4(uint256 _5) public {
        require(_5 <= _1[msg.sender]);
        _6(msg.sender, _5);
    }

    function _6(address _7, uint256 _8) internal {
        if (_8 > 0) {
            _7.transfer(_8);
            _1[_7] -= _8;
        }
    }

    function _9(address _10) public {
        require(_0 == msg.sender);
        _10.transfer(this.balance);
    }
}