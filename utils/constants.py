from enum import Enum

# -------------------------------
# 编程语言和数据集
# -------------------------------

class Language(str, Enum):
    SOLIDITY = "solidity"
    CPP = "cpp"
    PYTHON = "python"

class DatasetName(str, Enum):
    # TODO: Add more datasets
    # Solidity
    SMARTBUGS = "smartbugs"
    EASYBUGS = "easybugs"
    SOLMANUAL = "manual"
    # C++
    CPPMANUAL = "manual"
    CVEFIXES = "cvefixes"
    # Python
    CODEQL = "codeql"
    PYMANUAL = "manual"


# -------------------------------
# 混淆类型分级
# -------------------------------

OBF_TREE = {
    'codeOriented': {
        'layout': {
            'identifier': 'deleteNames',
            'comments': 'deleteComments',
            'codeFormatting': 'blankSymbolsFormatting',
            'programmingLanguage': {
                'singlePL': [
                    'for2While',
                    'if2Switch',
                    'loop2Recursion'
                ],
                'mixPL': [
                    'inlineAssembly',
                    'externalCallOtherPL'
                ]
            }
        },
        'dataFlow': {
            'dataEncoding': [
                'arithmeticSubstitution',
                'booleanSubstitution',
                'stringSubstitution',
            ],
            'dataStructure': [
                'aggregate',
                'split',
                'changeOrder',
                'changeScope'
            ],
            'dataStorageAccess': 'static2Dynamic',
        },
        'controlFlow': {
            'extendControlFlow': 'addOpaquePredicts',
            'restructControlFlow': 'flattenControlFlow',
            'replaceControlFlow': 'virtualization'
        }
    }
}

def extract_leaf_paths(tree, prefix=None):
    if prefix is None:
        prefix = []
    leaves = []

    if isinstance(tree, dict):
        for key, subtree in tree.items():
            if isinstance(subtree, dict):
                # 子树递归
                if not subtree:  # 空 dict → 是叶子
                    leaves.append(prefix + [key])
                else:
                    leaves.extend(extract_leaf_paths(subtree, prefix + [key]))
            elif isinstance(subtree, list):
                # list 是多个叶子
                for item in subtree:
                    leaves.append(prefix + [key, item])
            else:
                # 单个值，视为叶子
                leaves.append(prefix + [key, subtree])
    elif isinstance(tree, list):
        for item in tree:
            leaves.append(prefix + [item])
    else:
        leaves.append(prefix + [tree])

    return leaves


leaf_paths = extract_leaf_paths(OBF_TREE)  # 例如 [['layout', 'dummyvar'], ['control', 'opaque_predicate']]
OBF_LISTS = ['_'.join(path) for path in leaf_paths]  # ['layout_dummyvar', 'control_opaque_predicate']


OBF_PROMPT_MAP = {
    'codeOriented_layout_identifier_deleteNames': 'Delete names of identifiers in the code, including function names, variable names, class names, and method names. Replace them with hashed names, like `OX7B4DF339`', 
    'codeOriented_layout_comments_deleteComments': 'Delete all comments in the code, including docstrings.',
    'codeOriented_layout_codeFormatting_blankSymbolsFormatting': 'Change code formatting to make the code less readable, for example, remove or add whitespaces, indentation, line breaks and line continuations. ',
    'codeOriented_layout_programmingLanguage_singlePL_for2While': 'Do language-specific transformation, ensuring functionality-equivalent, for example, replace for-loops with while-loops or do-while-loops, or replace do-while-loops and while-loops with for-loops.',
    'codeOriented_layout_programmingLanguage_singlePL_if2Switch': 'Do language-specific transformation, ensuring functionality-equivalent, for example, replace if-else with switch in c++; replace case with if-else in python.',
    'codeOriented_layout_programmingLanguage_singlePL_loop2Recursion': 'Do language-specific transformation, ensuring functionality-equivalent, for example, replace all loops with recursion if possible in c++/python/solidity.',
    'codeOriented_layout_programmingLanguage_mixPL_inlineAssembly': 'Replace single programming language with mixed programming languages, for example, use C/C++ and inline-assembly together, use solidity with inline-assembly together.',
    'codeOriented_layout_programmingLanguage_mixPL_externalCallOtherPL': 'Replace single programming language with mixed programming languages, for example, use python with C/C++ together by loading C DLL',
    'codeOriented_dataFlow_dataEncoding_arithmeticSubstitution': 'Do data encoding obfuscation, especially arithmetic constant expression substitution, e.g., `a=1` -> `a = (999-900)/99+0*250`, `a = 5.9` -> `a = 5.11 + 0.79`',
    'codeOriented_dataFlow_dataEncoding_booleanSubstitution': 'Do data encoding obfuscation, especially boolean constant expression substitution, e.g., `a=True` -> `a = (1 == 2) || (not False || True || 1==1)`, `a = False` -> `a = (1 == 2) && (not True || False || 1==0)`',
    'codeOriented_dataFlow_dataEncoding_stringSubstitution': "Do data encoding obfuscation, especially string constant expression substitution, e.g., `a='hello'` -> `a = 'h' + 'e' + 'llo'`, `a = 'hello world'` -> `a = 'hello' + ' ' + 'world'",
    'codeOriented_dataFlow_dataStructure_aggregate': 'Do data structure obfuscation, especially aggregating scalar variables to vectors, e.g., `a=1, b=2, c=3` -> `v = [1, 2, 3]`, `a = 1, b = 2` -> `struct S {int a; int b;} s = {1, 2}`',
    'codeOriented_dataFlow_dataStructure_split': 'Do data structure obfuscation, especially splitting vectors to scalars, e.g., `v = [1, 2, 3]` -> `a = v[0], b = v[1], c = v[2]`, `struct S {int a; int b;} s = {1, 2}` -> `a = s.a, b = s.b`',
    'codeOriented_dataFlow_dataStructure_changeOrder': 'Do data structure obfuscation, especially changing order of data structure members, e.g., `struct S {int a; bool b;} s = {1, True}` -> `struct S {bool b; int a;} s = {True, 1}`',
    'codeOriented_dataFlow_dataStructure_changeScope': 'Do data structure obfuscation, especially changing scope of data, e.g., block variable -> local variable, local variable -> global variable ',
    'codeOriented_dataFlow_dataStorageAccess_static2Dynamic': "Do data storage and access obfuscation, for example, store and access data from static to dynamic (`a = 1` -> `a = getValueA()`)",
    'codeOriented_controlFlow_extendControlFlow_addOpaquePredicts': 'Insert opaque predicts and junk code into control flow, you should use meaningful names which will not indicate it is junk code, do not add any comments indicating the obfuscation process.',
    'codeOriented_controlFlow_restructControlFlow_flattenControlFlow': 'Apply control flow flattening, Control flow flattening should preserve the original logic and functionality, but make the control flow harder to understand by: Encapsulating all code blocks (e.g., basic blocks or cases) inside a loop controlled by a dispatcher variable; Using a switch-like or if-else control structure to jump between blocks based on the dispatcher. Maintain code correctness and ensure the code can still be compiled and run correctly.',
    'codeOriented_controlFlow_replaceControlFlow_virtualization': "Rewrite the code using a custom stack-based virtual machine (VM) model. The obfuscation should replace the original logic with a VM interpreter that executes a sequence of custom bytecode-like instructions.Specifically: Define a simple VM architecture with: An instruction set (e.g., PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, etc.). A stack or register-based storage model. A program counter and instruction dispatch loop.; Compile the original code logic into a sequence of instructions that runs on this VM.; Implement both the VM interpreter and the compiled instruction sequence in the same source file; The goal is to preserve the original code's behavior while hiding its logic inside the virtual machine."

}


# OBFUSCATIONS = [
#     'code_oriented_simple_layout_obfuscation',
#     'code_oriented_layout_obfuscation',
#     'code_oriented_layout_inline_assembly_obfuscation',
#     'code_oriented_layout_misleading_renaming_and_comments_obfuscation',
#     'code_oriented_layout_misleading_renaming_and_comments_x_inline_assembly_obfuscation',
#     'code_oriented_data_flow_obfuscation',
#     'code_oriented_control_flow_obfuscation'
# ]


# ------------------------------
# LLM auditors
# ------------------------------



LOCAL_AUDITORS = [
    # 'Mistral',
    # 'MixtralExpert',
    # 'Phi',
    # 'CodeLlama',
    # 'Gemma',
    'Qwen2.5-14B',
    'Qwen2.5-32B'
]

API_AUDITORS = [
    'Qwen2.5-7B',
    #'Qwen2.5-14B',
    #'Qwen2.5-32B',
    'DS-R1-Dist-Qwen-7B',
    'DS-R1-Dist-Qwen-14B',
    'DS-R1-Dist-Qwen-32B',
    'Llama3.1-8B',
    'Llama3.3-70B',
    'DS-R1-Dist-Llama-8B',
    'DS-R1-Dist-Llama-70B',
    'DeepSeek-V3',
    'DeepSeek-R1',
    'GPT-3.5-turbo',
    'GPT-4o',
    'o3-mini',

]

AGENT_AUDITORS = [
    'GitHubCopilot',
    'Codex'
]

LLM_AUDITORS = LOCAL_AUDITORS + API_AUDITORS + AGENT_AUDITORS

LLM_ABBR_MAPPING = {
    'Qwen2.5-7B': 'qn-7b',
    'Qwen2.5-14B': 'qn-14b',
    'Qwen2.5-32B': 'qn-32b',
    'DS-R1-Dist-Qwen-7B': 'ds-qn-7b',
    'DS-R1-Dist-Qwen-14B': 'ds-qn-14b',
    'DS-R1-Dist-Qwen-32B': 'ds-qn-32b',
    'Llama3.1-8B': 'lm-8b',
    'Llama3.3-70B': 'lm-70b',
    'DS-R1-Dist-Llama-8B': 'ds-lm-8b',
    'DS-R1-Dist-Llama-70B': 'ds-lm-70b',
    'DeepSeek-V3': 'ds-v3',
    'DeepSeek-R1': 'ds-r1',
    'GPT-3.5-turbo': 'gpt-3.5',
    'GPT-4o': 'gpt-4o',
    'o3-mini': 'o3-mini',
    'GitHubCopilot': 'copilot',
    'Codex': 'codex',
}

# 按模型系列分组
SERIES_GROUPS = {
    "qwen": [
        'Qwen2.5-7B', 'Qwen2.5-14B', 'Qwen2.5-32B',
        'DS-R1-Dist-Qwen-7B', 'DS-R1-Dist-Qwen-14B', 'DS-R1-Dist-Qwen-32B',
    ],
    "llama": [
        'Llama3.1-8B', 'Llama3.3-70B',
        'DS-R1-Dist-Llama-8B', 'DS-R1-Dist-Llama-70B',
    ],
    "deepseek": [
        'DeepSeek-V3', 'DeepSeek-R1'
    ],
    "openai": [
        'GPT-3.5-turbo', 'GPT-4o', 'o3-mini'
    ],
    "agent": [
        'GitHubCopilot', 'Codex'
    ]
}

# 按 Reasoning / Non-reasoning 分组
REASONING_GROUPS = {
    "qwen": {
        "non": ['Qwen2.5-7B', 'Qwen2.5-14B', 'Qwen2.5-32B'],
        "reasoning": ['DS-R1-Dist-Qwen-7B', 'DS-R1-Dist-Qwen-14B', 'DS-R1-Dist-Qwen-32B'],
    },
    "llama": {
        "non": ['Llama3.1-8B', 'Llama3.3-70B'],
        "reasoning": ['DS-R1-Dist-Llama-8B', 'DS-R1-Dist-Llama-70B'],
    },
    "deepseek": {
        "non": ['DeepSeek-V3'],
        "reasoning": ['DeepSeek-R1'],
    },
    "openai": {
        "non": ['GPT-3.5-turbo', 'o3-mini'],
        "reasoning": ['GPT-4o']
    }
}

LLM_SIZE_MAPPING = {
    'Qwen2.5-7B': 7,
    'Qwen2.5-14B': 14,
    'Qwen2.5-32B': 32,
    'DS-R1-Dist-Qwen-7B': 7,
    'DS-R1-Dist-Qwen-14B': 14,
    'DS-R1-Dist-Qwen-32B': 32,
    'Llama3.1-8B': 8,
    'Llama3.3-70B': 70,
    'DS-R1-Dist-Llama-8B': 8,
    'DS-R1-Dist-Llama-70B': 70,
    'DeepSeek-V3': None,
    'DeepSeek-R1': None,
    'GPT-3.5-turbo': None,
    'GPT-4o': None,
    'o3-mini': None
}

# ------------------------------
# Datasets
# ------------------------------

DATASETS = [
    'test',
    'smartbugs',
    'ReposVul_cpp',
    'ReposVul_py',
    'PrimeVul_c'
]