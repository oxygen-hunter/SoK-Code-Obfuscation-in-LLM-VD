def obfuscate_string(s, i=0):
    if i >= len(s):
        return ""
    if s[i].isalnum():
        return s[i] + obfuscate_string(s, i + 1)
    return obfuscate_string(s, i + 1)

__FOR_TESTING__ = obfuscate_string("5a5b6e17f9c34025a75b052fc06c50ab")