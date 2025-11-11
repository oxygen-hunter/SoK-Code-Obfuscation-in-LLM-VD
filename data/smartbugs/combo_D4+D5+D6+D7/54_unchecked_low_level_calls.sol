pragma solidity ^0.4.24;

contract demo {
    struct Data {
        address a;
        address b;
        address c;
        uint256 d;
    }

    function transfer(address _x, address _y, address[] _z, uint _w) public returns (bool) {
        require(_z.length > 0);
        bytes4 f = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint j = 0; j < _z.length; j++) {
            Data memory data = Data({a: _x, b: _y, c: _z[j], d: _w});
            data.b.call(f, data.a, data.c, data.d);
        }
        return true;
    }
}