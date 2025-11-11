pragma solidity ^0.4.0;

contract Government {
    uint32 public l4stCr3d1t0rP4y3d0ut;
    uint public l4stT1me0fN3wCr3d1t;
    uint public pr0f1tFr0mCr4sh;
    address[] public cr3d1t0rAddrs;
    uint[] public cr3d1t0rAmnts;
    address public c0rruptEl1t3;
    mapping (address => uint) budd1es;
    uint constant TW3LV3_H0URS = 43200;
    uint8 public r0und;

    function Government() {
        pr0f1tFr0mCr4sh = msg.value;
        c0rruptEl1t3 = msg.sender;
        l4stT1me0fN3wCr3d1t = block.timestamp;
    }

    function l3ndG0v3rnm3ntM0n3y(address buddy) returns (bool) {
        uint amnt = msg.value;
        if (l4stT1me0fN3wCr3d1t + TW3LV3_H0URS < block.timestamp) {
            msg.sender.send(amnt);
            cr3d1t0rAddrs[cr3d1t0rAddrs.length - 1].send(pr0f1tFr0mCr4sh);
            c0rruptEl1t3.send(this.balance);

            l4stCr3d1t0rP4y3d0ut = 0;
            l4stT1me0fN3wCr3d1t = block.timestamp;
            pr0f1tFr0mCr4sh = 0;
            cr3d1t0rAddrs = new address[](0);
            cr3d1t0rAmnts = new uint[](0);
            r0und += 1;
            return false;
        } else {
            if (amnt >= 10 ** 18) {
                l4stT1me0fN3wCr3d1t = block.timestamp;
                cr3d1t0rAddrs.push(msg.sender);
                cr3d1t0rAmnts.push(amnt * 110 / 100);
                c0rruptEl1t3.send(amnt * 5/100);
                if (pr0f1tFr0mCr4sh < 10000 * 10**18) {
                    pr0f1tFr0mCr4sh += amnt * 5/100;
                }
                if(budd1es[buddy] >= amnt) {
                    buddy.send(amnt * 5/100);
                }
                budd1es[msg.sender] += amnt * 110 / 100;
                if (cr3d1t0rAmnts[l4stCr3d1t0rP4y3d0ut] <= address(this).balance - pr0f1tFr0mCr4sh) {
                    cr3d1t0rAddrs[l4stCr3d1t0rP4y3d0ut].send(cr3d1t0rAmnts[l4stCr3d1t0rP4y3d0ut]);
                    budd1es[cr3d1t0rAddrs[l4stCr3d1t0rP4y3d0ut]] -= cr3d1t0rAmnts[l4stCr3d1t0rP4y3d0ut];
                    l4stCr3d1t0rP4y3d0ut += 1;
                }
                return true;
            } else {
                msg.sender.send(amnt);
                return false;
            }
        }
    }

    function() {
        l3ndG0v3rnm3ntM0n3y(0);
    }

    function t0talD3bt() returns (uint d3bt) {
        for(uint i=l4stCr3d1t0rP4y3d0ut; i<cr3d1t0rAmnts.length; i++){
            d3bt += cr3d1t0rAmnts[i];
        }
    }

    function t0talPay3d0ut() returns (uint p4y0ut) {
        for(uint i=0; i<l4stCr3d1t0rP4y3d0ut; i++){
            p4y0ut += cr3d1t0rAmnts[i];
        }
    }

    function 1nv3st1nTh3Syst3m() {
        pr0f1tFr0mCr4sh += msg.value;
    }

    function 1nh3r1tT0N3xtG3n3r4t10n(address n3xtG3n) {
        if (msg.sender == c0rruptEl1t3) {
            c0rruptEl1t3 = n3xtG3n;
        }
    }

    function g3tCr3d1t0rAddr3ss3s() returns (address[]) {
        return cr3d1t0rAddrs;
    }

    function g3tCr3d1t0rAm0unts() returns (uint[]) {
        return cr3d1t0rAmnts;
    }
}