import argparse
import json
import sys
import os
import pandas as pd
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor, as_completed

import torch
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from auditors.openrouterapi_auditor import OpenRouterAPIAuditor
from auditors.mistral_auditor import MistralAuditor
from auditors.mixtralexpert_auditor import MixtralExpertAuditor
from auditors.codellama_auditor import CodeLlamaAuditor
from auditors.gemma_auditor import GemmaAuditor
from auditors.phi_auditor import PhiAuditor
from auditors.qwen_auditor import Qwen_2p5_14B_Instruct_Auditor, Qwen_2p5_32B_Instruct_Auditor
from auditors.githubcopilot_auditor import CopilotAuditor
from auditors.codex_auditor import CodexAuditor
from utils.logger import logger, log_dot
from utils.constants import AGENT_AUDITORS, LLM_AUDITORS, LOCAL_AUDITORS, API_AUDITORS
from utils.commons import get_txt_content_as_str

def load_config():
    with open("config/secret_config.json", "r") as f:
        return json.load(f)
    
def read_combo_log(combo_mode):
    output_path = Path('logs')
    with open(output_path / f"combo_log_{combo_mode}.json", "r") as f:
        combo_log_map = json.load(f)
    all_combos = list(combo_log_map.keys())
    return all_combos
    
def init_auditor(auditor_name, max_new_tokens=4096, max_new_tokens_reasoing=2048, temperature=1e-5, use_flash_attn=False):
    model = None
    config = load_config()
    key = config["OPENROUTER_API_KEY"]
    proxy = config.get("PROXY")
    match auditor_name:
        case 'Qwen2.5-7B':
            model = OpenRouterAPIAuditor(model='qwen/qwen-2.5-7b-instruct', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'Qwen2.5-14B':
            model = Qwen_2p5_14B_Instruct_Auditor(max_new_tokens=max_new_tokens, temperature=temperature)
        case 'Qwen2.5-32B':
            #model = OpenRouterAPIAuditor(model='qwen/qwen-2.5-32b-instruct', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
            model = Qwen_2p5_32B_Instruct_Auditor(max_new_tokens=max_new_tokens, temperature=temperature)
        case 'DS-R1-Dist-Qwen-7B':
            model = OpenRouterAPIAuditor(model='deepseek/deepseek-r1-distill-qwen-7b', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'DS-R1-Dist-Qwen-14B':
            model = OpenRouterAPIAuditor(model='deepseek/deepseek-r1-distill-qwen-14b', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'DS-R1-Dist-Qwen-32B':
            model = OpenRouterAPIAuditor(model='deepseek/deepseek-r1-distill-qwen-32b', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'Llama3.1-8B':
            model = OpenRouterAPIAuditor(model='meta-llama/llama-3.1-8b-instruct', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'Llama3.3-70B':
            model = OpenRouterAPIAuditor(model='meta-llama/llama-3.3-70b-instruct', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'DS-R1-Dist-Llama-8B':
            model = OpenRouterAPIAuditor(model='deepseek/deepseek-r1-distill-llama-8b', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'DS-R1-Dist-Llama-70B':
            model = OpenRouterAPIAuditor(model='deepseek/deepseek-r1-distill-llama-70b', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'DeepSeek-V3':
            model = OpenRouterAPIAuditor(model='deepseek/deepseek-chat-v3-0324', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'DeepSeek-R1':
            model = OpenRouterAPIAuditor(model='deepseek/deepseek-r1', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'GPT-3.5-turbo':
            model = OpenRouterAPIAuditor(model='openai/gpt-3.5-turbo', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'GPT-4o':    
            model = OpenRouterAPIAuditor(model='openai/gpt-4o', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)
        case 'o3-mini':
            model = OpenRouterAPIAuditor(model='openai/o3-mini', api_key=key, proxy=proxy, max_new_tokens=max_new_tokens, temperature=temperature)

        case 'CodeLlama':
            model = CodeLlamaAuditor(max_new_tokens=max_new_tokens, temperature=temperature)
        case 'Gemma':
            model = GemmaAuditor(max_new_tokens=max_new_tokens, temperature=temperature)
        case 'Mistral':
            model = MistralAuditor(max_new_tokens=max_new_tokens, temperature=temperature)
        case 'MixtralExpert':
            model = MixtralExpertAuditor(max_new_tokens=max_new_tokens, temperature=temperature)
        case 'Phi':
            model = PhiAuditor(max_new_tokens=max_new_tokens, use_flash_attn=True, temperature=temperature)
        case 'GitHubCopilot':
            model = CopilotAuditor()
        case 'Codex':
            model = CodexAuditor()
    return model
        
def audit(model, todo_code_dir, result_csv_path, do):
    logger.info(f"Auditing code in directory: {todo_code_dir}")
    if not Path(todo_code_dir).exists():
        logger.error(f"Directory {todo_code_dir} does not exist.")
        return
    
    if Path(result_csv_path).exists():
        df = pd.read_csv(result_csv_path)
        df['key'] = df['file_name'].str.extract(r'^(.*?)_')

        # 按 key 分组并取第一条记录，每个 key 只保留一条
        grouped_df = df.groupby('key', as_index=False).first()

        # 设置 key 为索引后丢弃 'key' 列是不需要的，这里已经是 index
        result_dict = {
            row['key']: {
                'file_name': row['file_name'],
                'audit_report': row['audit_report'],
                'input_token_num': row['input_token_num'],
                'output_token_num': row['output_token_num'],
                'inference_time': row['inference_time']
            }
            for _, row in grouped_df.iterrows()
        }
        # 删掉audit_report为空或为Inference failed的项
        result_dict = {k: v for k, v in result_dict.items() if v['audit_report'] not in [None, '', 'Inference failed']}
    else:
        result_dict = {}

    if do == 'redo':
        result_dict = {}
    for path in Path(todo_code_dir).iterdir():
        if path.suffix in ['.sol', '.cpp', '.py', '.c']:
            # N_xxx.sol/.cpp/.py
            key_parts = path.stem.split('_')
            if len(key_parts) >= 2:
                key = key_parts[0]
                if key not in result_dict:
                    todo_code = get_txt_content_as_str(path)

                    # 调用audit函数并存储结果
                    (
                        result,
                        input_token_num,
                        output_token_num,
                        inference_time,
                        # queried_text,
                        # score,
                    ) = model.audit(code=todo_code)
                    result_dict[key] = {
                        'file_name': path.name,
                        'audit_report': result,
                        'input_token_num': input_token_num,
                        'output_token_num': output_token_num,
                        'inference_time': inference_time
                    }
                    #print('.', end='', flush=True)
                    if result.lower() == 'inference failed':
                        log_dot(logger, dot='x')
                    else:
                        log_dot(logger)
            else:
                logger.error(f"filename format is wrong {path}")
        else:
            logger.error(f"unknown suffix: {path.suffix}")
    
    # sort the result_dict by id
    result_dict = dict(sorted(result_dict.items(), key=lambda item:int(item[0])))
    df = pd.DataFrame.from_dict(result_dict, orient='index')
    Path(result_csv_path).parent.mkdir(parents=True, exist_ok=True)
    df.to_csv(result_csv_path, index=False)
    #print('\n')
    logger.info(f'\nAudit result saved to: {result_csv_path}')
    
def audit_one_dir(auditor_name, dataset, dir_name, do):
    """单个 auditor 审计单个目录的函数"""
    model = init_auditor(auditor_name)  # 模型初始化放这里避免多进程不能共享
    dataset_dir = Path('data') / dataset
    todo_code_dir = dataset_dir / dir_name
    result_csv_path = Path('results') / dataset / 'audit_results' / auditor_name / f'{dir_name}.csv'
    audit(model, todo_code_dir, result_csv_path, do)
    torch.cuda.empty_cache()
    return dir_name  # 可用于 logging

def audit_one_auditor(auditor_name, dataset, combo_mode, do):
    logger.info(f"[{auditor_name}] Starting auditor")

    dataset_dir = Path('data') / dataset
    todo_code_dirs = [dir for dir in dataset_dir.iterdir() if dir.is_dir() and dir.name not in ['explanation']]

    all_combos = read_combo_log(combo_mode)
    todo_code_dirs = [dir.name for dir in todo_code_dirs if dir.name in all_combos or dir.name == 'original']

    # 内层并行：多个目录并行审计（你也可以改成 ProcessPoolExecutor）
    with ThreadPoolExecutor(max_workers=min(4, len(todo_code_dirs))) as executor:
        futures = {
            executor.submit(audit_one_dir, auditor_name, dataset, dir_name, do): dir_name
            for dir_name in todo_code_dirs
        }

        for future in as_completed(futures):
            dir_name = futures[future]
            try:
                future.result()
                logger.info(f"[{auditor_name}] Completed {dir_name}")
            except Exception as e:
                logger.error(f"[{auditor_name}] Failed {dir_name} with exception: {e}")

    logger.info(f"[{auditor_name}] Completed all dirs in dataset: {dataset}")


def batch_audit_multiple_thread(auditor_names, dataset, combo_mode, do):
    if 'all' in auditor_names:
        auditor_names = LLM_AUDITORS
    if 'local' in auditor_names:
        auditor_names = LOCAL_AUDITORS
    if 'api' in auditor_names:
        auditor_names = API_AUDITORS
    if 'agent' in auditor_names:
        auditor_names = AGENT_AUDITORS

    with ProcessPoolExecutor(max_workers=min(4, len(auditor_names))) as executor:
        futures = {
            executor.submit(audit_one_auditor, name, dataset, combo_mode, do): name
            for name in auditor_names
        }

        for future in as_completed(futures):
            name = futures[future]
            try:
                future.result()
            except Exception as e:
                logger.error(f"[{name}] Auditor failed with exception: {e}")

def batch_audit_single_thread(auditor_names, dataset, combo_mode, do):
    if 'all' in auditor_names:
        auditor_names = LLM_AUDITORS
    if 'local' in auditor_names:
        auditor_names = LOCAL_AUDITORS
    if 'api' in auditor_names:
        auditor_names = API_AUDITORS
    if 'agent' in auditor_names:
        auditor_names = AGENT_AUDITORS
    print(auditor_names)
    for auditor_name in auditor_names:
        model = init_auditor(auditor_name)
        if model is None:
            logger.error(f"Model {auditor_name} not recognized.")
            return
        logger.info(f"Using {auditor_name} for auditing.")

        # audit all obfuscations defined in combo_mode
        dataset_dir = Path(f'data') / dataset
        todo_code_dirs = [dir for dir in dataset_dir.iterdir() if dir.is_dir() and dir.name not in ['explanation']]
        # 
        all_combos = read_combo_log(combo_mode)
        todo_code_dirs = [dir for dir in todo_code_dirs if dir.name in all_combos or dir.name == 'original']
        print(todo_code_dirs)
        for dir in todo_code_dirs:
            todo_code_dir = dataset_dir / dir.name
            result_csv_path = Path(f'results') / dataset / 'audit_results' / auditor_name / f'{dir.name}.csv'
            audit(model, todo_code_dir, result_csv_path, do)
            logger.info(f"Auditing {auditor_name} for {dataset}.{dir.name} completed.")
        logger.info(f"Auditing {auditor_name} for {dataset} completed.")

        torch.cuda.empty_cache()


def main(args):
    # 解析命令行参数
    auditor_names = args.auditors
    dataset = args.dataset
    combo_mode = args.combo_mode
    do = args.do

    if 'local' in auditor_names or auditor_names[0] in LOCAL_AUDITORS:
        # 执行批量审计
        batch_audit_single_thread(auditor_names, dataset, combo_mode, do)
    elif 'api' in auditor_names or auditor_names[0] in API_AUDITORS:
        batch_audit_multiple_thread(auditor_names, dataset, combo_mode, do)
    elif 'agent' in auditor_names or auditor_names[0] in AGENT_AUDITORS:
        batch_audit_single_thread(auditor_names, dataset, combo_mode, do)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Audit tool runner")

    parser.add_argument('--auditors', nargs='+', type=str, required=True, help='List of auditor names to run')
    parser.add_argument('--dataset', type=str, required=True, help='Dataset name to be used')
    parser.add_argument('--combo_mode', type=str, required=True, help='Obfuscation combo mode')
    parser.add_argument('--do', type=str, required=True, choices=['redo', 'append'], help='redo or append')

    args = parser.parse_args()

    times = 5
    while (times > 0):
        main(args)
        times -= 1