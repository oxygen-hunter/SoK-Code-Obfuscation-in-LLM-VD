pragma solidity ^0.4.19;

contract OX7B4DF339 {
    mapping (address => uint) OX9A3D12F7;
    uint OX5E2A6B9C;

    function OX1F4C89AB() public {
        uint OX8D6E5F4A = OX9A3D12F7[msg.sender];
        if (OX8D6E5F4A > 0) {
            OX5E2A6B9C -= OX8D6E5F4A;
             
            bool OX2B7C9D3E = msg.sender.call.value(OX8D6E5F4A)();
            require (OX2B7C9D3E);
            OX9A3D12F7[msg.sender] = 0;
        }
    }

    function OX3E7A9B2F() public payable {
        OX9A3D12F7[msg.sender] += msg.value;
        OX5E2A6B9C += msg.value;
    }
}