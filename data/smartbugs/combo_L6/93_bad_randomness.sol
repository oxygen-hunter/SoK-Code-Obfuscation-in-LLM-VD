pragma solidity ^0.4.0;

contract LuckyDoubler {
    address private O;
    uint private B = 0;
    uint private F = 5;
    uint private M = 125;
    mapping (address => U) private UMap;
    E[] private EArr;
    uint[] private UEArr;
    
    function LuckyDoubler() {
        O = msg.sender;
    }

    modifier OMod { if (msg.sender == O) _; }

    struct U {
        address I;
        uint D;
        uint P;
    }

    struct E {
        address EA;
        uint D;
        uint P;
        bool PStatus;
    }

    function() { I(); }

    function I() private {
        if (msg.value < 1 ether) {
            msg.sender.send(msg.value);
            return;
        }
        J();
    }

    function J() private {
        uint DV = 1 ether;
        if (msg.value > 1 ether) {
            msg.sender.send(msg.value - 1 ether);
            DV = 1 ether;
        }
        if (UMap[msg.sender].I == address(0)) {
            UMap[msg.sender].I = msg.sender;
            UMap[msg.sender].D = 0;
            UMap[msg.sender].P = 0;
        }
        EArr.push(E(msg.sender, DV, (DV * (M) / 100), false));
        UMap[msg.sender].D++;
        UEArr.push(EArr.length - 1);
        B += (DV * (100 - F)) / 100;
        uint I = UEArr.length > 1 ? R(UEArr.length) : 0;
        E memory EInst = EArr[UEArr[I]];
        if (B > EInst.P) {
            uint P = EInst.P;
            EInst.EA.send(P);
            EInst.PStatus = true;
            UMap[EInst.EA].P++;
            B -= P;
            if (I < UEArr.length - 1)
                UEArr[I] = UEArr[UEArr.length - 1];
            UEArr.length--;
        }
        uint FS = this.balance - B;
        if (FS > 0) {
            O.send(FS);
        }
    }

    uint256 constant private FACTOR = 1157920892373161954235709850086879078532699846656405640394575840079131296399;
    function R(uint max) constant private returns (uint256 result) {
        uint256 F = FACTOR * 100 / max;
        uint256 LBN = block.number - 1;
        uint256 HV = uint256(block.blockhash(LBN));
        return uint256((uint256(HV) / F)) % max;
    }

    function CO(address NO) OMod { O = NO; }

    function CM(uint MVal) OMod {
        if (MVal < 110 || MVal > 150) throw;
        M = MVal;
    }

    function CF(uint NF) OMod {
        if (F > 5) throw;
        F = NF;
    }

    function MF() constant returns (uint F, string I) {
        F = M;
        I = 'The current multiplier applied to all deposits. Min 110%, max 150%.';
    }

    function CFee() constant returns (uint FP, string I) {
        FP = F;
        I = 'The fee percentage applied to all deposits. It can change to speed payouts (max 5%).';
    }

    function TE() constant returns (uint C, string I) {
        C = EArr.length;
        I = 'The number of deposits.';
    }

    function US(address UAddr) constant returns (uint D, uint P, string I) {
        if (UMap[UAddr].I != address(0x0)) {
            D = UMap[UAddr].D;
            P = UMap[UAddr].P;
            I = 'Users stats: total deposits, payouts received.';
        }
    }

    function ED(uint I) constant returns (address U, uint P, bool PS, string I) {
        if (I < EArr.length) {
            U = EArr[I].EA;
            P = EArr[I].P / 1 finney;
            PS = EArr[I].PStatus;
            I = 'Entry info: user address, expected payout in Finneys, payout status.';
        }
    }
}