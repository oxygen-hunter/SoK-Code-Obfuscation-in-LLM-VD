import logging
import os
from datetime import datetime

logger = logging.getLogger("llm_obfuscater")
logger.setLevel(logging.INFO)

formatter = logging.Formatter("[%(asctime)s] [%(levelname)s] %(message)s")

# 控制台输出
console_handler = logging.StreamHandler()
console_handler.setFormatter(formatter)
logger.addHandler(console_handler)

# 日志文件输出，文件名带日期时间
log_dir = "logs"
os.makedirs(log_dir, exist_ok=True)

# 生成带时间戳的日志文件名
timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
log_filename = f"obfuscater_{timestamp}.log"
log_path = os.path.join(log_dir, log_filename)

file_handler = logging.FileHandler(log_path, encoding='utf-8')
file_handler.setFormatter(formatter)
logger.addHandler(file_handler)

# def log_dot(logger):
#     if logger.handlers:
#         logger.handlers[0].stream.write('.')
#         logger.handlers[0].stream.flush()

def log_dot(logger, dot='.'):
    for handler in logger.handlers:
        try:
            handler.flush()
            handler.stream.write(dot)
            handler.flush()
        except Exception as e:
            pass  # 某些 handler（比如网络 logger）可能不支持 .stream.write
