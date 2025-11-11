bytes32 constant __FOR_TESTING__ = 0x5a5b6e17f9c34025a75b052fc06c50ab;

assembly {
    function getTestValue() -> val {
        val := __FOR_TESTING__
    }
}