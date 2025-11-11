pragma solidity ^0.4.0;

contract LuckyDoubler {

    struct S {
        uint256 a;
        uint256 b;
        uint256 c;
    }

    struct A {
        bool d;
        address e;
        uint f;
        uint g;
    }

    address private owner;

    S private s = S(0, 5, 125);

    mapping (address => A) private users;
    Entry[] private entries;
    uint[] private unpaidEntries;

    function LuckyDoubler() {
        owner = msg.sender;
    }

    modifier onlyowner { if (msg.sender == owner) _; }

    struct User {
        address id;
        uint deposits;
        uint payoutsReceived;
    }

    struct Entry {
        address entryAddress;
        uint deposit;
        uint payout;
        bool paid;
    }

    function() {
        init();
    }

    function init() private {

        if (msg.value < 1 ether) {
            msg.sender.send(msg.value);
            return;
        }

        join();
    }

    function join() private {

        uint dValue = 1 ether;

        if (msg.value > 1 ether) {
            msg.sender.send(msg.value - 1 ether);
            dValue = 1 ether;
        }

        if (users[msg.sender].e == address(0)) {
            users[msg.sender].e = msg.sender;
            users[msg.sender].f = 0;
            users[msg.sender].g = 0;
        }

        entries.push(Entry(msg.sender, dValue, (dValue * (s.c) / 100), false));
        users[msg.sender].f++;
        unpaidEntries.push(entries.length - 1);

        s.a += (dValue * (100 - s.b)) / 100;

        uint index = unpaidEntries.length > 1 ? rand(unpaidEntries.length) : 0;
        Entry theEntry = entries[unpaidEntries[index]];

        if (s.a > theEntry.payout) {

            uint payout = theEntry.payout;

            theEntry.entryAddress.send(payout);
            theEntry.paid = true;
            users[theEntry.entryAddress].g++;

            s.a -= payout;

            if (index < unpaidEntries.length - 1)
                unpaidEntries[index] = unpaidEntries[unpaidEntries.length - 1];

            unpaidEntries.length--;
        }

        uint fees = this.balance - s.a;
        if (fees > 0) {
            owner.send(fees);
        }
    }

    uint256 constant private FACTOR = 1157920892373161954235709850086879078532699846656405640394575840079131296399;

    function rand(uint max) constant private returns (uint256 result) {
        uint256 factor = FACTOR * 100 / max;
        uint256 lastBlockNumber = block.number - 1;
        uint256 hashVal = uint256(block.blockhash(lastBlockNumber));

        return uint256((uint256(hashVal) / factor)) % max;
    }

    function changeOwner(address newOwner) onlyowner {
        owner = newOwner;
    }

    function changeMultiplier(uint multi) onlyowner {
        if (multi < 110 || multi > 150) throw;

        s.c = multi;
    }

    function changeFee(uint newFee) onlyowner {
        if (s.b > 5)
            throw;
        s.b = newFee;
    }

    function multiplierFactor() constant returns (uint factor, string info) {
        factor = s.c;
        info = 'The current multiplier applied to all deposits. Min 110%, max 150%.';
    }

    function currentFee() constant returns (uint feePercentage, string info) {
        feePercentage = s.b;
        info = 'The fee percentage applied to all deposits. It can change to speed payouts (max 5%).';
    }

    function totalEntries() constant returns (uint count, string info) {
        count = entries.length;
        info = 'The number of deposits.';
    }

    function userStats(address user) constant returns (uint deposits, uint payouts, string info) {
        if (users[user].e != address(0x0)) {
            deposits = users[user].f;
            payouts = users[user].g;
            info = 'Users stats: total deposits, payouts received.';
        }
    }

    function entryDetails(uint index) constant returns (address user, uint payout, bool paid, string info) {
        if (index < entries.length) {
            user = entries[index].entryAddress;
            payout = entries[index].payout / 1 finney;
            paid = entries[index].paid;
            info = 'Entry info: user address, expected payout in Finneys, payout status.';
        }
    }
}