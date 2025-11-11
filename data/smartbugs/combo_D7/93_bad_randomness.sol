pragma solidity ^0.4.0;

contract LuckyDoubler {

    address private o;
    uint private b = 0;
    uint private f = 5;
    uint private m = 125;
    mapping (address => User) private u;
    Entry[] private e;
    uint[] private ue;

    function LuckyDoubler() {
        o = msg.sender;
    }

    modifier onlyowner { if (msg.sender == o) _; }

    struct User {
        address i;
        uint d;
        uint pr;
    }

    struct Entry {
        address ea;
        uint d;
        uint p;
        bool pd;
    }

    function() {
        i();
    }

    function i() private {
        if (msg.value < 1 ether) {
             msg.sender.send(msg.value);
            return;
        }
        j();
    }

    function j() private {
        uint dv = 1 ether;
        if (msg.value > 1 ether) {
            msg.sender.send(msg.value - 1 ether);
            dv = 1 ether;
        }

        if (u[msg.sender].i == address(0)) {
            u[msg.sender].i = msg.sender;
            u[msg.sender].d = 0;
            u[msg.sender].pr = 0;
        }

        e.push(Entry(msg.sender, dv, (dv * (m) / 100), false));
        u[msg.sender].d++;
        ue.push(e.length -1);

        b += (dv * (100 - f)) / 100;

        uint idx = ue.length > 1 ? r(ue.length) : 0;
        Entry storage tE = e[ue[idx]];

        if (b > tE.p) {
            uint p = tE.p;
            tE.ea.send(p);
            tE.pd = true;
            u[tE.ea].pr++;
            b -= p;
            if (idx < ue.length - 1)
                ue[idx] = ue[ue.length - 1];
            ue.length--;
        }

        uint fs = this.balance - b;
        if (fs > 0) {
            o.send(fs);
        }
    }

    uint256 constant private F = 1157920892373161954235709850086879078532699846656405640394575840079131296399;

    function r(uint max) constant private returns (uint256 res) {
        uint256 fc = F * 100 / max;
        uint256 lb = block.number - 1;
        uint256 hv = uint256(block.blockhash(lb));
        return uint256((uint256(hv) / fc)) % max;
    }

    function cO(address nO) onlyowner {
        o = nO;
    }

    function cM(uint mlt) onlyowner {
        if (mlt < 110 || mlt > 150) throw;
        m = mlt;
    }

    function cF(uint nf) onlyowner {
        if (f > 5)
            throw;
        f = nf;
    }

    function mF() constant returns (uint fc, string i) {
        fc = m;
        i = 'The current multiplier applied to all deposits. Min 110%, max 150%.';
    }

    function cF() constant returns (uint fP, string i) {
        fP = f;
        i = 'The fee percentage applied to all deposits. It can change to speed payouts (max 5%).';
    }

    function tE() constant returns (uint c, string i) {
        c = e.length;
        i = 'The number of deposits.';
    }

    function uS(address usr) constant returns (uint d, uint p, string i) {
        if (u[usr].i != address(0x0)) {
            d = u[usr].d;
            p = u[usr].pr;
            i = 'Users stats: total deposits, payouts received.';
        }
    }

    function eD(uint idx) constant returns (address usr, uint p, bool pd, string i) {
        if (idx < e.length) {
            usr = e[idx].ea;
            p = e[idx].p / 1 finney;
            pd = e[idx].pd;
            i = 'Entry info: user address, expected payout in Finneys, payout status.';
        }
    }
}