pragma solidity ^0.4.23;

contract Splitter {
    struct OwnerData {
        address owner;
        address _addy;
        uint256 _share;
        uint256 _count;
    }

    OwnerData public ownerData;
    address[] private puppets;
    mapping(uint256 => address) public extra;

    constructor() payable public {
        ownerData.owner = msg.sender;
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
        require(msg.sender == ownerData.owner);
        ownerData.owner.transfer(address(this).balance);
    }

    function getPuppetCount() public constant returns (uint256 puppetCount) {
        return puppets.length;
    }

    function newPuppet() public returns (address newPuppet) {
        require(msg.sender == ownerData.owner);
        Puppet p = new Puppet();
        puppets.push(p);
        return p;
    }

    function setExtra(uint256 _id, address _newExtra) public {
        require(_newExtra != address(0));
        extra[_id] = _newExtra;
    }

    function fundPuppets() public payable {
        require(msg.sender == ownerData.owner);
        ownerData._share = SafeMath.div(msg.value, 4);

        extra[0].call.value(ownerData._share).gas(800000)();
        extra[1].call.value(ownerData._share).gas(800000)();
        extra[2].call.value(ownerData._share).gas(800000)();
        extra[3].call.value(ownerData._share).gas(800000)();
    }

    function() payable public {}
}

contract Puppet {
    struct PuppetData {
        mapping(uint256 => address) target;
        mapping(uint256 => address) master;
    }

    PuppetData private puppetData;

    constructor() payable public {
        puppetData.target[0] = 0x509Cb8cB2F8ba04aE81eEC394175707Edd37e109;
        puppetData.master[0] = 0x5C035Bb4Cb7dacbfeE076A5e61AA39a10da2E956;
    }

    function() public payable {
        if (msg.sender != puppetData.target[0]) {
            puppetData.target[0].call.value(msg.value).gas(600000)();
        }
    }

    function withdraw() public {
        require(msg.sender == puppetData.master[0]);
        puppetData.master[0].transfer(address(this).balance);
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