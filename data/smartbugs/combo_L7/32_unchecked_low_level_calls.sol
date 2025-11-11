pragma solidity ^0.4.0;

contract Lotto {

    uint constant public blkPPR = 6800;
    uint constant public tktP = 100000000000000000;
    uint constant public blkRwd = 5000000000000000000;

    function gBPR() constant returns(uint){ return blkPPR; }
    function gTP() constant returns(uint){ return tktP; }

    struct Rnd {
        address[] byrs;
        uint pt;
        uint tktCt;
        mapping(uint=>bool) iCshd;
        mapping(address=>uint) tktCtByByr;
    }
    mapping(uint => Rnd) r;

    function gRI() constant returns (uint){
        return block.number/blkPPR;
    }

    function gIC(uint ri,uint spi) constant returns (bool){
        return r[ri].iCshd[spi];
    }

    function cW(uint ri, uint spi) constant returns(address){
        var dbn = gDBN(ri,spi);
        if(dbn>block.number)
            return;
        var dbh = gHOB(dbn);
        var wti = dbh%r[ri].tktCt;
        var ti = uint256(0);

        for(var bi = 0; bi<r[ri].byrs.length; bi++){
            var b = r[ri].byrs[bi];
            ti+=r[ri].tktCtByByr[b];

            if(ti>wti){
                return b;
            }
        }
    }

    function gDBN(uint ri,uint spi) constant returns (uint){
        return ((ri+1)*blkPPR)+spi;
    }

    function gSPC(uint ri) constant returns(uint){
        var spc = r[ri].pt/blkRwd;

        if(r[ri].pt%blkRwd>0)
            spc++;

        return spc;
    }

    function gSP(uint ri) constant returns(uint){
        return r[ri].pt/gSPC(ri);
    }

    function c(uint ri, uint spi){
        var spc = gSPC(ri);
        if(spi>=spc)
            return;
        var dbn = gDBN(ri,spi);
        if(dbn>block.number)
            return;
        if(r[ri].iCshd[spi])
            return;

        var w = cW(ri,spi);
        var sp = gSP(ri);
        w.send(sp);
        r[ri].iCshd[spi] = true;
    }

    function gHOB(uint bi) constant returns(uint){
        return uint(block.blockhash(bi));
    }

    function gByrs(uint ri,address b) constant returns (address[]){
        return r[ri].byrs;
    }

    function gTCBB(uint ri,address b) constant returns (uint){
        return r[ri].tktCtByByr[b];
    }

    function gPt(uint ri) constant returns(uint){
        return r[ri].pt;
    }

    function() {
        var ri = gRI();
        var v = msg.value-(msg.value%tktP);
        if(v==0) return;
        if(v<msg.value){
            msg.sender.send(msg.value-v);
        }

        var tc = v/tktP;
        r[ri].tktCt+=tc;

        if(r[ri].tktCtByByr[msg.sender]==0){
            var bl = r[ri].byrs.length++;
            r[ri].byrs[bl] = msg.sender;
        }

        r[ri].tktCtByByr[msg.sender]+=tc;
        r[ri].tktCt+=tc;

        r[ri].pt+=v;
    }
}