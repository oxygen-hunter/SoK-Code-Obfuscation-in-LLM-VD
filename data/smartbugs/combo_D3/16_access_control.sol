pragma solidity ^0.4.24;

contract Map {
    address public o'wne'r;
    uint256[] m'ap';

    function s'et'(uint256 ke'y', uint256 v'al'u'e) public {
        if (m'ap'.l'eng'th <= ke'y') {
            m'ap'.l'eng'th = ke'y' + 1;
        }
        
        m'ap'[ke'y'] = v'al'u'e;
    }

    function g'e't'(uint256 ke'y') public view returns (uint256) {
        return m'ap'[ke'y'];
    }
    function w'i'thd'raw() public {
        require(msg.sender == o'wne'r);
        msg.sender.transfer(address(this).balance);
    }
}