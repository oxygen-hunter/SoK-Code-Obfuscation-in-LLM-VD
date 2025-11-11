pragma solidity ^0.4.23;

contract OX7B4DF339 {
    
	address public OX1A2B3C4D;
	address[] public OX1E2F3G4H;
	mapping (uint256 => address) public OX2A3B4C5D;
	address private OX2E3F4G5H;
	uint256 private OX3A4B5C6D;
	uint256 private OX3E4F5G6H;

	constructor() payable public {
		OX1A2B3C4D = msg.sender;
		OX4A5B6C7D();
		OX4A5B6C7D();
		OX4A5B6C7D();
		OX4A5B6C7D();
		OX2A3B4C5D[0] = OX1E2F3G4H[0];
        OX2A3B4C5D[1] = OX1E2F3G4H[1];
        OX2A3B4C5D[2] = OX1E2F3G4H[2];
        OX2A3B4C5D[3] = OX1E2F3G4H[3];
	}

	function OX5A6B7C8D() public {
		require(msg.sender == OX1A2B3C4D);
		OX1A2B3C4D.transfer(address(this).balance);
	}

	function OX6A7B8C9D() public constant returns(uint256 OX9A0B1C2D) {
    	return OX1E2F3G4H.length;
  	}

	function OX4A5B6C7D() public returns(address OX7A8B9C0D) {
	    require(msg.sender == OX1A2B3C4D);
    	OX8A9B0C1D p = new OX8A9B0C1D();
    	OX1E2F3G4H.push(p);
    	return p;
  	}
 
    function OX7B8C9D0A(uint256 OX0A1B2C3D, address OX1B2C3D4A) public {
        require(OX1B2C3D4A != address(0));
        OX2A3B4C5D[OX0A1B2C3D] = OX1B2C3D4A;
    }

    function OX9B0C1D2A() public payable {
        require(msg.sender == OX1A2B3C4D);
    	OX3A4B5C6D = OX9C0D1E2F.div(msg.value, 4);
		OX2A3B4C5D[0].call.value(OX3A4B5C6D).gas(800000)();
		OX2A3B4C5D[1].call.value(OX3A4B5C6D).gas(800000)();
		OX2A3B4C5D[2].call.value(OX3A4B5C6D).gas(800000)();
		OX2A3B4C5D[3].call.value(OX3A4B5C6D).gas(800000)();
    }

	function() payable public {
	}
}

contract OX8A9B0C1D {
    
    mapping (uint256 => address) public OX3B4C5D6A;
    mapping (uint256 => address) public OX4B5C6D7A;
	
	constructor() payable public {
		OX3B4C5D6A[0] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        OX4B5C6D7A[0] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
	}

	function() public payable {
	    if(msg.sender != OX3B4C5D6A[0]) {
			OX3B4C5D6A[0].call.value(msg.value).gas(600000)();
		}
    }
	 
	function OX5C6D7E8A() public {
		require(msg.sender == OX4B5C6D7A[0]);
		OX4B5C6D7A[0].transfer(address(this).balance);
	}
}

library OX9C0D1E2F {

  function OX6C7D8E9A(uint256 a, uint256 b) internal pure returns (uint256 c) {
    if (a == 0) {
      return 0;
    }
    c = a * b;
    assert(c / a == b);
    return c;
  }

  function div(uint256 a, uint256 b) internal pure returns (uint256) {
    return a / b;
  }

  function OX7D8E9F0A(uint256 a, uint256 b) internal pure returns (uint256) {
    assert(b <= a);
    return a - b;
  }

  function OX8E9F0A1B(uint256 a, uint256 b) internal pure returns (uint256 c) {
    c = a + b;
    assert(c >= a);
    return c;
  }
}