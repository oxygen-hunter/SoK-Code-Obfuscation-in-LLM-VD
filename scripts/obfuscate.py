import argparse
import json
import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from obfuscaters.llm_obfuscater import *
from utils.commons import get_txt_content_as_str
from pathlib import Path
from itertools import combinations
from collections import defaultdict
from utils.logger import logger
#from utils.constants import OBFUSCATIONS
from utils.constants import OBF_TREE, OBF_LISTS

def load_config():
    with open("config/secret_config.json", "r") as f:
        return json.load(f)
    
def init_obfuscater()->LLMObfuscater:
    config = load_config()
    key = config["OPENROUTER_API_KEY"]
    proxy = config.get("PROXY")
    return LLMObfuscater(api_key=key, proxy=proxy)

def remove_code_block_fences(input_str):
    lines = input_str.strip().splitlines()
    if len(lines) >= 2 and lines[0].startswith("```") and lines[-1].startswith("```"):
        lines = lines[1:-1]
    return "\n".join(lines)

def obfuscate_single_file(obfuscater:LLMObfuscater, obf_types, code_path, output_path):
    code = get_txt_content_as_str(code_path)
    obf_code = obfuscater.obfuscate(code, obf_types)
    obf_code = remove_code_block_fences(obf_code)
    Path(output_path).parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(obf_code)
        logger.info(f"Obfucated code saved to {output_path}")


def generate_short_map(obf_lists):
    counters = defaultdict(int)
    short_map = {}

    for full_name in obf_lists:
        if "layout" in full_name:
            prefix = "L"
        elif "dataFlow" in full_name:
            prefix = "D"
        elif "controlFlow" in full_name:
            prefix = "C"
        else:
            prefix = "X"

        counters[prefix] += 1
        short_map[full_name] = f"{prefix}{counters[prefix]}"

    return short_map

# 用于输出组合名，例如 combo_001_L1+C2
def get_combo_name(combo, short_map):
    short_ids = [short_map[item] for item in combo]
    return f"combo_{'+'.join(short_ids)}"

# 保存映射和组合日志
def save_logs(short_map, all_combos, combo_mode, output_dir="logs"):
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    with open(output_path / f"short_map_{combo_mode}.json", "w") as f:
        json.dump(short_map, f, indent=2)

    combo_log_map = {}
    with open(output_path / f"combo_log_{combo_mode}.json", "w") as f:
        for idx, combo in enumerate(all_combos):
            combo_name = get_combo_name(combo, short_map)
            combo_log_map[combo_name] = combo
        json.dump(combo_log_map, f, indent=2)


# 示例：生成所有双组合（可改成三组合等）
def generate_pair_combos(obf_lists):
    return list(combinations(obf_lists, 2))


def batch_obfuscate(combo_mode, code_path, output_path, do):

    if combo_mode == 'single':
        all_combos = [[obf] for obf in OBF_LISTS]
    elif combo_mode == 'combo_layout':
        all_combos = [[obf for obf in OBF_LISTS if "layout" in obf]]
    elif combo_mode == 'combo_dataFlow':
        all_combos = [[obf for obf in OBF_LISTS if "dataFlow" in obf]]
    elif combo_mode == 'combo_controlFlow':
        all_combos = [[obf for obf in OBF_LISTS if "controlFlow" in obf]]
    elif combo_mode == 'single_layout':
        all_combos = [[obf] for obf in OBF_LISTS if "layout" in obf]
    elif combo_mode == 'single_dataFlow':
        all_combos = [[obf] for obf in OBF_LISTS if "dataFlow" in obf]
    elif combo_mode == 'single_controlFlow':
        all_combos = [[obf] for obf in OBF_LISTS if "controlFlow" in obf]
    
    
    elif combo_mode == 'diy-1': 
        # layout按如下组合
        # - singlePL 三合一
        # - 其他全部单体
        # dataFlow按如下组合
        # - single-PL 三合一
        # - dataEncoding 三合一
        # - dataStructure 四合一
        # controlFlow全部单体
        layout_combos = [[obf] for obf in OBF_LISTS if "layout" in obf and "singlePL" not in obf]
        layout_combos.append([obf for obf in OBF_LISTS if "layout" in obf and "singlePL" in obf])

        data_encoding_combos = [[obf for obf in OBF_LISTS if "dataEncoding" in obf]]
        data_structure_combos = [[obf for obf in OBF_LISTS if "dataStructure" in obf]]
        data_storageAccess_combos = [[obf] for obf in OBF_LISTS if "dataStorageAccess" in obf]
        data_combos = data_encoding_combos + data_structure_combos + data_storageAccess_combos
        
        control_combos = [[obf] for obf in OBF_LISTS if "controlFlow" in obf]
        
        all_combos = layout_combos + data_combos + control_combos
    else:
        raise ValueError(f"Unknown combo mode: {combo_mode}")

    obfuscater = init_obfuscater()
    obf_short_map = generate_short_map(OBF_LISTS)
    save_logs(obf_short_map, all_combos, combo_mode)
    
    for combo in all_combos:
        combo_name = get_combo_name(combo, obf_short_map)
        if not Path(code_path).exists():
            logger.error(f"Input code path {code_path} does not exist.")
            return
        # batch for dir
        if Path(code_path).is_dir():
            for file in Path(code_path).iterdir():
                if file.suffix in ['.sol', '.cpp', '.py', '.c']:
                    output_file = Path(output_path) / combo_name / file.relative_to(code_path)
                    if not output_file.exists() or do == 'redo' or get_txt_content_as_str(output_file) == "Obfuscation failed":
                        obfuscate_single_file(obfuscater, list(combo), file, output_file)
                    else:
                        logger.info(f"Output path {output_file} already exists. Skipping obfuscation.")
        else: # single for file
            if not Path(output_path).exists() or do == 'redo':
                obfuscate_single_file(obfuscater, code_path, output_path)
        

def main(args):
    batch_obfuscate(args.combo_mode, args.code_path, args.output_path, args.do)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Obfuscater')

    parser.add_argument('--combo_mode', type=str, required=True, help='Obfuscation combo mode')
    parser.add_argument('--code_path', type=str, required=True, help='Path to the code file to be obfuscated')
    parser.add_argument('--output_path', type=str, required=True, help='Path to save the obfuscated code')
    parser.add_argument('--do', type=str, required=True, choices=['redo', 'append'], help='redo or append'
    )

    args = parser.parse_args()

    times = 5
    while (times > 0):
        main(args)
        times -= 1