pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address public OXE9E7C5A7;
    uint256[] OX4F2A6C3B;

    function OX1AC3F7D8(uint256 OX5B8E2F6D, uint256 OX8A9D3E1F) public {
        if (OX4F2A6C3B.length <= OX5B8E2F6D) {
            OX4F2A6C3B.length = OX5B8E2F6D + 1;
        }
        
        OX4F2A6C3B[OX5B8E2F6D] = OX8A9D3E1F;
    }

    function OX9D8E3F6C(uint256 OX2A7F5D8E) public view returns (uint256) {
        return OX4F2A6C3B[OX2A7F5D8E];
    }
    function OX3B7E6F9A() public {
        require(msg.sender == OXE9E7C5A7);
        msg.sender.transfer(address(this).balance);
    }
}