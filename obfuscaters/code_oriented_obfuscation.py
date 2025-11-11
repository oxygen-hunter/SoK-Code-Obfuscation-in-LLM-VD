layout_obfuscation = \
"""
Modify the code by the following steps:
1. Delete names of identifiers in the code, including function names, variable names, class names, and method names. Replace them with hashed names, like `OX7B4DF339`

2. Delete all comments in the code, including docstrings.

3. Change code formatting to make the code less readable, for example, remove or add whitespaces, indentation, line breaks and line continuations. 

4. Do language-specific transformation, ensuring functionality-equivalent, for example, replace for-loops with while-loops, replace if-else with switch in c++/solidity; replace case with if-else in python.

5. Replace single programming language with mixed programming languages, for example, use C/C++ and inline-assembly together, use python with C/C++ together, use solidity with inline-assembly together.

code:

"""

data_flow_obfuscation = \
"""
Modify the code by the following steps:
1. Do data encoding obfuscation, for example
- arithmetic constant expression substitution, e.g., `a=1` -> `a = (999-900)/99+0*250`, `a = 5.9` -> `a = 5.11 + 0.79`
- boolean constant expression substitution, e.g., `a=True` -> `a = (1 == 2) || (not False || True || 1==1)`, `a = False` -> `a = (1 == 2) && (not True || False || 1==0)`
- string constant expression substitution, e.g., `a='hello'` -> `a = 'h' + 'e' + 'llo'`, `a = 'hello world'` -> `a = 'hello' + ' ' + 'world'

2. Do data structure obfuscation, for example
- aggregate scalar variables to vectors, e.g., `a=1, b=2, c=3` -> `v = [1, 2, 3]`, `a = 1, b = 2` -> `struct S {int a; int b;} s = {1, 2}`
- split vectors to scalars, e.g., `v = [1, 2, 3]` -> `a = v[0], b = v[1], c = v[2]`, `struct S {int a; int b;} s = {1, 2}` -> `a = s.a, b = s.b`
- change order of data structure members, e.g., `struct S {int a; bool b;} s = {1, True}` -> `struct S {bool b; int a;} s = {True, 1}`
- change scope of data, e.g., block variable -> local variable, local variable -> global variable 

3. Do data storage and access obfuscation, for example
- store and access data from static to dynamic (`a = 1` -> `a = getValueA()`)

code:

"""

data_flow_obfuscation = \
"""
Modify the code by the following steps:
1. change data encoding, for example
- arithmetic constant expression substitution, e.g., `a=1` -> `a = (999-900)/99+0*250`, `a = 5.9` -> `a = 5.11 + 0.79`
- boolean constant expression substitution, e.g., `a=True` -> `a = (1 == 2) || (not False || True || 1==1)`, `a = False` -> `a = (1 == 2) && (not True || False || 1==0)`
- string constant expression substitution, e.g., `a='hello'` -> `a = 'h' + 'e' + 'llo'`, `a = 'hello world'` -> `a = 'hello' + ' ' + 'world'

2. change data structure, for example
- aggregate scalar variables to vectors, e.g., `a=1, b=2, c=3` -> `v = [1, 2, 3]`, `a = 1, b = 2` -> `struct S {int a; int b;} s = {1, 2}`
- split vectors to scalars, e.g., `v = [1, 2, 3]` -> `a = v[0], b = v[1], c = v[2]`, `struct S {int a; int b;} s = {1, 2}` -> `a = s.a, b = s.b`
- change order of data structure members, e.g., `struct S {int a; bool b;} s = {1, True}` -> `struct S {bool b; int a;} s = {True, 1}`
- change scope of data, e.g., block variable -> local variable, local variable -> global variable 

3. change data storage and access, for example
- store and access data from static to dynamic (`a = 1` -> `a = getValueA()`)

code:

"""

control_flow_obfuscation = \
"""
Modify the code by the following steps:
1. extend control flow, for example
- insert opaque predicts and junk code into control flow 

2. restructure control flow, for example
- flatten control flow

3. replace control flow, for example
- virtualization (or any method that encrypt source code and decrypt it in run-time)

code:
"""