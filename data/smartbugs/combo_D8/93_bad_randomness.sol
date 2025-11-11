pragma solidity ^0.4.0;

contract LuckyDoubler {

    address private o;
    uint private b = 0;
    uint private f = getFee();
    uint private m = getMultiplier();

    mapping (address => User) private usrs;
    Entry[] private e;
    uint[] private ue;

    function getFee() private pure returns (uint) {
        return 5;
    }

    function getMultiplier() private pure returns (uint) {
        return 125;
    }

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
        uint dep;
        uint po;
        bool p;
    }

    function() {
        i();
    }

    function i() private{
        if (msg.value < 1 ether) {
            msg.sender.send(msg.value);
            return;
        }
        j();
    }

    function j() private {
        uint dV = 1 ether;
        if (msg.value > 1 ether) {
            msg.sender.send(msg.value - 1 ether);
            dV = 1 ether;
        }

        if (usrs[msg.sender].i == address(0)) {
            usrs[msg.sender].i = msg.sender;
            usrs[msg.sender].d = 0;
            usrs[msg.sender].pr = 0;
        }

        e.push(Entry(msg.sender, dV, (dV * (m) / 100), false));
        usrs[msg.sender].d++;
        ue.push(e.length -1);

        b += (dV * (100 - f)) / 100;

        uint idx = ue.length > 1 ? r(ue.length) : 0;
        Entry tE = e[ue[idx]];

        if (b > tE.po) {
            uint po = tE.po;
            tE.ea.send(po);
            tE.p = true;
            usrs[tE.ea].pr++;
            b -= po;

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
    function r(uint mx) constant private returns (uint256 res){
        uint256 fac = F * 100 / mx;
        uint256 lbn = block.number - 1;
        uint256 hV = uint256(block.blockhash(lbn));
        return uint256((uint256(hV) / fac)) % mx;
    }

    function co(address nO) onlyowner {
        o = nO;
    }

    function cm(uint mu) onlyowner {
        if (mu < 110 || mu > 150) throw;
        m = mu;
    }

    function cf(uint nf) onlyowner {
        if (f > 5)
            throw;
        f = nf;
    }

    function mf() constant returns (uint factor, string info) {
        factor = m;
        info = 'The current multiplier applied to all deposits. Min 110%, max 150%.';
    }

    function cf() constant returns (uint fp, string info) {
        fp = f;
        info = 'The fee percentage applied to all deposits. It can change to speed payouts (max 5%).';
    }

    function tE() constant returns (uint c, string info) {
        c = e.length;
        info = 'The number of deposits.';
    }

    function uS(address u) constant returns (uint d, uint p, string info) {
        if (usrs[u].i != address(0x0)) {
            d = usrs[u].d;
            p = usrs[u].pr;
            info = 'Users stats: total deposits, payouts received.';
        }
    }

    function eD(uint idx) constant returns (address u, uint p, bool pd, string info) {
        if (idx < e.length) {
            u = e[idx].ea;
            p = e[idx].po / 1 finney;
            pd = e[idx].p;
            info = 'Entry info: user address, expected payout in Finneys, payout status.';
        }
    }
}