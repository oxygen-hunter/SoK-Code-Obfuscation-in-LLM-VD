pragma solidity ^0.4.24;

contract OX7B4DF339 {
  mapping (address => uint) public OX3F2F5C2A;
  string constant OX1A2B3C4D = "Nu Token";

  function OX8C9D7E6F() OX5A6B7C8D OX9E8F7D6C public {
    OX3F2F5C2A[msg.sender] += 20;
  }

  modifier OX5A6B7C8D() {
    require(keccak256(abi.encodePacked("Nu Token")) == OX4C3B2A1D(msg.sender).OX2E1F0D3C());
    _;
  }

  modifier OX9E8F7D6C {
      require(OX3F2F5C2A[msg.sender] == 0);
      _;
  }
}

contract OX4C3B2A1D {
    function OX2E1F0D3C() external pure returns(bytes32) {
        return(keccak256(abi.encodePacked("Nu Token")));
    }
}

contract OX0F1E2D3C {
    bool OX6F5E4D3C;
    function OX2E1F0D3C() external returns(bytes32) {
        if(!OX6F5E4D3C) {
            OX6F5E4D3C = true;
            OX7B4DF339(msg.sender).OX8C9D7E6F();
        }
        return(keccak256(abi.encodePacked("Nu Token")));
    }
    function OX7D6C5B4A(address OX5C4B3A2D) public {
        OX7B4DF339(OX5C4B3A2D).OX8C9D7E6F();
    }
}