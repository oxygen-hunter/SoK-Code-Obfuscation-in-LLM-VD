import os
from abc import ABC, abstractmethod
import time
from openai import OpenAI
from utils.logger import logger

class BaseAuditor(ABC):
    def __init__(
        self, 
        use_rag: bool = False
    ):
        self.use_rag = use_rag

    @abstractmethod
    def inference(self, system_prompt, user_prompt) -> str:
        """
        使用LLM进行推理，返回结果
        :param system_prompt: 系统提示
        :param user_prompt: 用户提示
        :return: 推理结果
        """
        pass


    def build_system_prompt(self) -> str:
        return "You are an intelligent code auditor designed for dectecting the security vulnerabilities for the given code."

    def build_user_prompt(self, code: str) -> str:
        return f"""
        Detect the security vulnerabilities in the following code.
        Provide your results in the following format:
        1. Whether the code is vulnerable or not. Tell me YES or NO.
        2. If YES, please list the vulnerabilities type, the vulnerable lines, and the reason.
        3. If NO, please tell me the reason.
        4. If you are not sure, please tell me the reason.

        Code: 
        {code}
        """.strip()


    def audit(self, code: str) -> str:
        """
        分析代码中的潜在漏洞，返回格式应包括：
        - 漏洞行号（可多个）
        - 漏洞类型
        - 原因解释
        """
        system_prompt = self.build_system_prompt()
        user_prompt = self.build_user_prompt(code)
        
        result = self.inference(system_prompt, user_prompt)
        return result
        
    
    def rewrite(self, code: str) -> str:
        """
        重写代码，返回格式应包括：
        - 重写后的代码
        """
        system_prompt = "You are an code expert designed for rewriting code to make it more secure."
        user_prompt = f"""
        Rewrite the following code to make it more secure. Give me the rewritten code, with a explanation of the changes made.
        Code: 
        {code}
        """.strip()
        result = self.inference(system_prompt, user_prompt)
        return result