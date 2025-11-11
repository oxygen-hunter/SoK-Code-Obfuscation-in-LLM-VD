import os
from abc import ABC, abstractmethod
import time
from typing import List
from openai import OpenAI
import httpx
from utils.logger import logger
from utils.constants import OBF_PROMPT_MAP


# ---------------------------
# 基类定义
# ---------------------------
class LLMObfuscater(ABC):
    def __init__(self, api_key: str, proxy: str = None, model: str = "gpt-4o"):
        self.api_key = api_key
        self.model = model
        self.client = OpenAI(
            api_key=self.api_key,
            base_url="https://openrouter.ai/api/v1",
            http_client=httpx.Client(proxy=proxy) if proxy else None
        )
        logger.info(f"LLMObfuscater initialized with model: {self.model}")

    

    def generate_prompt(self, code: str, obf_types: list[str]) -> str:
        instructions = []
        for i, obf_type in enumerate(obf_types, start=1):
            if obf_type not in OBF_PROMPT_MAP:
                raise ValueError(f"Unknown obfuscation method: {obf_type}")
            instructions.append(f"{i}. {OBF_PROMPT_MAP[obf_type]}")

        instruction_block = "\n".join(instructions)
        prompt = (
            "Modify the given code by the following instructions:\n"
            f"{instruction_block}\n\n"
            "--------------------------------\n"
            "Code:\n"
            f"{code}"
            "--------------------------------\n"
            "Your answer must contain only the modified code! Do not explan anything extra! Wrap the output code with ```language\n and ```\n, where language is the programming language of the code, like python, c, c++, solidity, etc.\n"
        )
        return prompt



    def obfuscate(self, code: str, obf_types: list[str]) -> str:
        #prompt = self.build_prompt(code)
        user_prompt = self.generate_prompt(code, obf_types)
        try:
            response = self.client.chat.completions.create(
                model=self.model,
                messages=[
                    {"role": "system", "content": "You are an expert code obfuscater. Your task is to rewrite the given code by the given instructions, to make it less readable while preserving its functionality. Maintain code correctness and ensure the code can still be compiled and run correctly. Do not add any comments indicating the obfuscation process."},
                    {"role": "user", "content": user_prompt}
                ],
                temperature=0.7,
            )
            logger.info("Successfully obfuscated one code sample.")
            return response.choices[0].message.content.strip()
        except Exception as e:
            logger.warning(f"First obfuscation attempt failed: {e}. Retrying after 10s...")
            time.sleep(10)
            try:
                response = self.client.chat.completions.create(
                    model=self.model,
                    messages=[
                        {"role": "system", "content": "You are an expert code obfuscater. Your task is to rewrite the given code by the given instructions, to make it less readable while preserving its functionality. Maintain code correctness and ensure the code can still be compiled and run correctly. Do not add any comments indicating the obfuscation process."},
                        {"role": "user", "content": user_prompt}
                    ],
                    temperature=0.7,
                )
                result = response.choices[0].message.content.strip()
                logger.info("Successfully obfuscated one code sample after retry.")
                return result
            except Exception as e:
                logger.error(f"Second obfuscation attempt failed: {e}")
                return "Obfuscation failed"
    

# # ---------------------------
# # 示例子类：code-oriented layout 
# # ---------------------------
# class SimpleLayoutObfuscater(LLMObfuscater):
#     def build_prompt(self, code: str) -> str:
#         return f"""
# Modify the code by the following steps:
# 1. Delete names of identifiers in the code, including function names, variable names, class names, and method names. Replace them with hashed names, like `OX7B4DF339`

# 2. Delete all comments in the code, including docstrings.


# Your answer must contain only the modified code! Do not wrap code with anything like ```! Do not explan anything extra!

# code:
# ```
# {code}
# ```
#         """.strip()

# # ---------------------------
# # 示例子类：code-oriented layout 
# # ---------------------------
# class LayoutObfuscater(LLMObfuscater):
#     def build_prompt(self, code: str) -> str:
#         return f"""
# Modify the code by the following steps:
# 1. Delete names of identifiers in the code, including function names, variable names, class names, and method names. Replace them with hashed names, like `OX7B4DF339`

# 2. Delete all comments in the code, including docstrings.

# 3. Change code formatting to make the code less readable, for example, remove or add whitespaces, indentation, line breaks and line continuations. 

# 4. Do language-specific transformation, ensuring functionality-equivalent, for example, replace for-loops with while-loops, replace if-else with switch in c++/solidity; replace case with if-else in python.

# 5. Replace single programming language with mixed programming languages, for example, use C/C++ and inline-assembly together, use python with C/C++ together, use solidity with inline-assembly together.

# Your answer must contain only the modified code! Do not wrap code with anything like ```! Do not explan anything extra!

# code:
# ```
# {code}
# ```
#         """.strip()



# class Layout_Inline_Assembly_Obfuscater(LLMObfuscater):
#     def build_prompt(self, code: str) -> str:
#         return f"""
# Modify the code by the following steps:
# 1. Use inline-assembly to obfuscate each function in the code (for C/C++ or Solidity language), ensuring functionality-equivalent.

# Your answer must contain only the modified code! Do not wrap code with anything like ```! Do not explan anything extra!

# code:
# ```
# {code}
# ```
#         """.strip()
    

# class Layout_Misleading_Renaming_and_Comments_Obfuscater(LLMObfuscater):
#     def build_prompt(self, code: str) -> str:
#         return f"""
# Modify the code by the following steps:

# 1. Use inline-assembly to obfuscate each function in the code (for C/C++ or Solidity language), ensuring functionality-equivalent.

# 2. Change names of identifiers in the code, including function names, variable names, class names, and method names. Replace them with meaningful but misleading names, make sure the names are not related to the code logic, like `howManyQueens`, `Counts`, `DepositOneEther`.

# 3. Add misleading comments that matches the names of identifiers, make sure the comments are not related to the code logic. For example, `// Calculate the sum of two numbers` for a function that actually calculates the product of two numbers.

# Your answer must contain only the modified code! Do not wrap code with anything like ```! Do not explan anything extra!

# code:
# ```
# {code}
# ```
#         """.strip()


# class Layout_Misleading_Renaming_and_Comments_x_inline_assembly_Obfuscater(LLMObfuscater):
#     def build_prompt(self, code: str) -> str:
#         return f"""
# Modify the code by the following steps:
# 1. Change names of identifiers in the code, including function names, variable names, class names, and method names. Replace them with meaningful but misleading names, make sure the names are not related to the code logic, like `howManyQueens`, `Counts`, `DepositOneEther`.

# 2. Add misleading comments that matches the names of identifiers, make sure the comments are not related to the code logic. For example, `// Calculate the sum of two numbers` for a function that actually calculates the product of two numbers.

# Your answer must contain only the modified code! Do not wrap code with anything like ```! Do not explan anything extra!

# code:
# ```
# {code}
# ```
#         """.strip()


# # ---------------------------
# # 示例子类：code oriented data flow 
# # ---------------------------
# class DataFlowObfuscater(LLMObfuscater):
#     def build_prompt(self, code: str) -> str:
#         return """
# Modify the code by the following steps:
# 1. Do data encoding obfuscation, for example
# - arithmetic constant expression substitution, e.g., `a=1` -> `a = (999-900)/99+0*250`, `a = 5.9` -> `a = 5.11 + 0.79`
# - boolean constant expression substitution, e.g., `a=True` -> `a = (1 == 2) || (not False || True || 1==1)`, `a = False` -> `a = (1 == 2) && (not True || False || 1==0)`
# - string constant expression substitution, e.g., `a='hello'` -> `a = 'h' + 'e' + 'llo'`, `a = 'hello world'` -> `a = 'hello' + ' ' + 'world'

# 2. Do data structure obfuscation, for example
# - aggregate scalar variables to vectors, e.g., `a=1, b=2, c=3` -> `v = [1, 2, 3]`, `a = 1, b = 2` -> `struct S {int a; int b;} s = {1, 2}`
# - split vectors to scalars, e.g., `v = [1, 2, 3]` -> `a = v[0], b = v[1], c = v[2]`, `struct S {int a; int b;} s = {1, 2}` -> `a = s.a, b = s.b`
# - change order of data structure members, e.g., `struct S {int a; bool b;} s = {1, True}` -> `struct S {bool b; int a;} s = {True, 1}`
# - change scope of data, e.g., block variable -> local variable, local variable -> global variable 

# 3. Do data storage and access obfuscation, for example
# - store and access data from static to dynamic (`a = 1` -> `a = getValueA()`)

# Your answer must contain only the modified code! Do not wrap code with anything like ```! Do not explan anything extra!

# """+f"""
# code:
# ```
# {code}
# ```
#         """.strip()
    


# # ---------------------------
# # 示例子类：code oriented control flow 
# # ---------------------------
# class ControlFlowObfuscater(LLMObfuscater):
#     def build_prompt(self, code: str) -> str:
#         return """
# Modify the code by the following steps:
# 1. insert opaque predicts and junk code into control flow 

# 2. flatten control flow

# Ensure that the obfuscated code is functionally equivalent to the original code.

# Do not add any comments indicating the obfuscation process.

# Your answer must contain only the modified code! Do not wrap code with anything like ```! Do not explan anything extra!

# """+f"""
# code:
# ```
# {code}
# ```
#         """.strip()