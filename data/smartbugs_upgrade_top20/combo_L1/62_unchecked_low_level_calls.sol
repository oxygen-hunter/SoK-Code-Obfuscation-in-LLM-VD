pragma solidity ^0.4.19;
contract OX7B4DF339 {
    function OX7F2E16A3(address OX3C8E3FB1, uint OX4A9E7F56) returns (bool OX5FDFD7A2);
    function OX5C3B6E11(address OX1F2C9F3A) constant returns (uint OX7D9B8CAF);
}
contract OX2A6B9D47 {
    address OX1B3D9F24;
    function OX2A6B9D47() {
        OX1B3D9F24 = msg.sender;
    }
    function OX5F3A7D9E(address OX6C8F9B53) public {
        OX7B4DF339 OX1E5A2C8D = OX7B4DF339(OX6C8F9B53);
        OX1E5A2C8D.OX7F2E16A3(OX1B3D9F24, OX1E5A2C8D.OX5C3B6E11(this));
    }
    function OX9C6E8B4A() public {
        OX1B3D9F24.transfer(this.balance);
    }
    function OX4E7B9D5C(uint OX3F8A9C6D, address OX2D9E7F3B) public {
        for(uint OX5A7C3D9B = 0; OX5A7C3D9B < OX3F8A9C6D; OX5A7C3D9B++){
            OX2D9E7F3B.call.value(0 wei)();
        }
    }
}