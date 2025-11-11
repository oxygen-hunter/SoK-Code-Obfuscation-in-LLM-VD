pragma solidity ^0.4.16;

contract OX7B4DF339 {
    struct OX1A2B3C4D {
        address OX5E6F7G8H;
        uint OX9I0J1K2L;
    }

    event OX3M4N5O6P(
        uint OX9I0J1K2L,
        uint OXQ7R8S9T,
        address OXU0V1W2X,
        address OXY3Z4A5B,
        address OXC6D7E8F,
        uint OXG9H0I1J,
        bytes32 OXK2L3M4N
    );

    event OXO5P6Q7R(
        uint OX9I0J1K2L,
        address OX1A2B3C4,
        uint OXQ7R8S9T
    );

    event OXS8T9U0V(
        uint OX9I0J1K2L,
        address OX1A2B3C4,
        uint OXQ7R8S9T
    );

    uint public constant OXW1X2Y3Z = 2.5 ether;
    uint public constant OXA4B5C6D = 0.03 ether;
    uint public constant OXE7F8G9H = 50;
    uint public constant OXI0J1K2L = (OXW1X2Y3Z + OXA4B5C6D) / OXE7F8G9H;  
    address OXM3N4O5P;

    bool public OXQ6R7S8T = false;
    uint public OXU9V0W1X = 1;
    uint public OXY2Z3A4B = block.number;
    uint OXC5D6E7F = 0;
    mapping (uint => OX1A2B3C4D) OXG8H9I0J;
    uint[] OXK1L2M3N;

    function OX7B4DF339() public {
        OXM3N4O5P = msg.sender;
    }

    function () payable public {
        OXO5P6Q7R();
    }

    function OXO5P6Q7R() payable public {
        if (OXQ6R7S8T) {
            msg.sender.transfer(msg.value);
            return;
        }

        uint OXO4P5Q6R = msg.value;

        while (OXO4P5Q6R >= OXI0J1K2L && OXC5D6E7F < OXE7F8G9H) {
            uint OXS7T8U9V = 0;
            if (OXK1L2M3N.length > 0) {
                OXS7T8U9V = OXK1L2M3N[OXK1L2M3N.length-1];
                OXK1L2M3N.length--;
            } else {
                OXS7T8U9V = OXC5D6E7F++;
            }

            OXG8H9I0J[OXS7T8U9V] = OX1A2B3C4D(msg.sender, OXU9V0W1X);
            OXO5P6Q7R(OXU9V0W1X, msg.sender, OXS7T8U9V);
            OXO4P5Q6R -= OXI0J1K2L;
        }

        if (OXC5D6E7F == OXE7F8G9H) {
            OX3M4N5O6P();
        }

        if (OXO4P5Q6R > 0) {
            msg.sender.transfer(OXO4P5Q6R);
        }
    }

    function OX3M4N5O6P() private {
        address OXY3Z4A5B = OXG8H9I0J[uint(block.coinbase) % OXE7F8G9H].OX5E6F7G8H;
        address OXC6D7E8F = OXG8H9I0J[uint(msg.sender) % OXE7F8G9H].OX5E6F7G8H;
        uint OXG9H0I1J = block.difficulty;
        bytes32 OXK2L3M4N = keccak256(OXY3Z4A5B, OXC6D7E8F, OXG9H0I1J);

        uint OXQ7R8S9T = uint(OXK2L3M4N) % OXE7F8G9H;
        address OXU0V1W2X = OXG8H9I0J[OXQ7R8S9T].OX5E6F7G8H;
        OX3M4N5O6P(OXU9V0W1X, OXQ7R8S9T, OXU0V1W2X, OXY3Z4A5B, OXC6D7E8F, OXG9H0I1J, OXK2L3M4N);

        OXU9V0W1X++;
        OXC5D6E7F = 0;
        OXY2Z3A4B = block.number;

        OXU0V1W2X.transfer(OXW1X2Y3Z);
        OXM3N4O5P.transfer(OXA4B5C6D);
    }

    function OXS8T9U0V() public {
        uint OXW1X2Y3Z = 0;
        for (uint OXJ4K5L6M = 0; OXJ4K5L6M < OXE7F8G9H; OXJ4K5L6M++) {
            if (msg.sender == OXG8H9I0J[OXJ4K5L6M].OX5E6F7G8H && OXU9V0W1X == OXG8H9I0J[OXJ4K5L6M].OX9I0J1K2L) {
                OXW1X2Y3Z += OXI0J1K2L;
                OXG8H9I0J[OXJ4K5L6M] = OX1A2B3C4D(address(0), 0);
                OXK1L2M3N.push(OXJ4K5L6M);
                OXS8T9U0V(OXU9V0W1X, msg.sender, OXJ4K5L6M);
            }
        }

        if (OXW1X2Y3Z > 0) {
            msg.sender.transfer(OXW1X2Y3Z);
        }
    }

    function OXV2W3X4Y() public {
        if (msg.sender == OXM3N4O5P) {
            OXQ6R7S8T = true;

            for (uint OXJ4K5L6M = 0; OXJ4K5L6M < OXE7F8G9H; OXJ4K5L6M++) {
                if (OXU9V0W1X == OXG8H9I0J[OXJ4K5L6M].OX9I0J1K2L) {
                    OXS8T9U0V(OXU9V0W1X, OXG8H9I0J[OXJ4K5L6M].OX5E6F7G8H, OXJ4K5L6M);
                    OXG8H9I0J[OXJ4K5L6M].OX5E6F7G8H.transfer(OXI0J1K2L);
                }
            }

            OX3M4N5O6P(OXU9V0W1X, OXE7F8G9H, address(0), address(0), address(0), 0, 0);
            OXU9V0W1X++;
            OXC5D6E7F = 0;
            OXY2Z3A4B = block.number;
            OXK1L2M3N.length = 0;
        }
    }

    function OXZ5A6B7C() public {
        if (msg.sender == OXM3N4O5P) {
            OXQ6R7S8T = !OXQ6R7S8T;
        }
    }

    function OXD8E9F0G() public {
        if (msg.sender == OXM3N4O5P) {
            selfdestruct(OXM3N4O5P);
        }
    }
}