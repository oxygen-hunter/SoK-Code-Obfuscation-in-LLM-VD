import os
import re
import subprocess
import tempfile
import time
from utils.logger import logger
from auditors.base_auditor import BaseAuditor

class CodexAuditor(BaseAuditor):
    def __init__(self):
        pass

    import re

    


    def inference(self, system_prompt, user_prompt):
        # Copilot CLI 暂不支持 system prompt / user prompt 拆开传，所以拼接
        #time.sleep(10)
        start_time = time.time()
        full_prompt = f"{system_prompt}\n\n{user_prompt}"

        def normalize_prompt(s: str) -> str:
            """
            清理 prompt 字符串，适合在 Windows cmd 下通过 -p 传给 Copilot
            处理：
            1. 换行符 (\n, \r, \u2028, \u2029) → 空格
            2. 制表符 (\t) → 空格
            3. 开头/结尾 Unicode 隐藏空白字符 → strip
            4. 双引号 " → \"
            5. Windows shell 控制字符 (& | < > ^ %) → 用空格或替换
            6. 删除不可打印控制字符 (0x00 ~ 0x1F 除 \t)
            """
            if not s:
                return ""
            
            # 保护 '\n'
            s = re.sub(r"'\n'", "__ESCAPED_NEWLINE_SINGLE__", s)

            #s = re.sub(r'"\n"', "__ESCAPED_NEWLINE_DOUBLE__", s)

            # 1. 替换换行符
            s = re.sub(r'[\r\n\u2028\u2029]+', ' ', s)

            # 2. 替换制表符
            s = s.replace('\t', ' ')

            # 3. 去除开头/结尾空白字符
            s = s.strip()

            
            # 反斜杠转义
            s = s.replace('\\', '\\\\')
            # 4. 转义双引号
            #s = s.replace('"', '\\"')

            s = s.replace('"', '""')
            

            # 5. 替换 Windows shell 特殊字符
            #    & | < > ^ % 这些在命令行会拆分或解析
            #s = re.sub(r'[&|<>^%]', ' ', s)

            # 6. 删除其他不可打印控制字符（ASCII 0x00 ~ 0x1F，保留空格）
            #s = re.sub(r'[\x00-\x08\x0b\x0c\x0e-\x1f]', '', s)

            # 7. 合并多余空格
            s = re.sub(r'\s+', ' ', s)

            # 8. 恢复显式 '\n' 
            s = s.replace("__ESCAPED_NEWLINE_SINGLE__", r"'\n'")
            #s = s.replace("__ESCAPED_NEWLINE_DOUBLE__", r'\\"\n\\"')

            return s

        #print(repr(full_prompt))
        #print("-----")
        full_prompt = normalize_prompt(full_prompt)
        print(full_prompt)
        print("-----")
        print(repr(full_prompt))
        #exit(1)
        cmd = f'codex exec "{full_prompt}" --skip-git-repo-check'
        #cmd = f'echo "{full_prompt}"'

        try:
            result = subprocess.run(
                cmd,
                shell=True,
                capture_output=True,
                text=True,
                encoding='utf-8',  # 明确指定UTF-8编码
                errors='replace',    # 忽略无法解码的字符
                timeout=120
            )
            inference_time = time.time() - start_time
            # 优先 stdout, 出错时 stderr
            print(result.stdout)
            print(result.stderr)
            if result.stdout:
                return result.stdout, 1, 1, inference_time
            else:
                logger.error(f"Inference error stderr: {result.stderr}")
                return 'Inference failed', 1, 1, inference_time
        except subprocess.TimeoutExpired:
            try:
                result = subprocess.run(
                    cmd,
                    shell=True,
                    capture_output=True,
                    text=True,
                    encoding='utf-8',  # 明确指定UTF-8编码
                    errors='replace',    # 忽略无法解码的字符
                    timeout=240
                )
                inference_time = time.time() - start_time
                # 优先 stdout, 出错时 stderr
                if result.stdout:
                    return result.stdout, 1, 1, inference_time
                else:
                    return 'Inference failed', 1, 1, inference_time
            except Exception as e:
                logger.error(f"Inference exception \n{e}")
                inference_time = time.time() - start_time
                return 'Inference failed', 1, 1, inference_time
        except Exception as e:
            logger.error(f"Inference exception \n{e}")
            inference_time = time.time() - start_time
            return 'Inference failed', 1, 1, inference_time
