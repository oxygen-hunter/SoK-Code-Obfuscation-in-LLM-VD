import argparse
import json
from pathlib import Path
import sys
import os
import concurrent.futures
import pandas as pd
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from utils.commons import get_txt_content_as_str
from evaluators.llm_evaluator import Evaluator
from utils.logger import logger, log_dot
from utils.constants import LLM_AUDITORS, LOCAL_AUDITORS, API_AUDITORS, AGENT_AUDITORS, DATASETS


def load_config():
    with open("config/secret_config.json", "r") as f:
        return json.load(f)
    

def init_evaluator()->Evaluator:
    config = load_config()
    key = config["OPENROUTER_API_KEY"]
    proxy = config.get("PROXY")
    #model = Evaluator(model='openai/gpt-4o', api_key=key, proxy=proxy)
    #model = Evaluator(model='meta-llama/llama-3.1-8b-instruct', api_key=key, proxy=proxy)
    #model = Evaluator(model='qwen/qwen-2.5-7b-instruct', api_key=key, proxy=proxy)
    model = Evaluator(model='meta-llama/llama-3.3-70b-instruct', api_key=key, proxy=proxy)
    
    
    return model


def index_gt_description_as_str(dataset, file_name)->str:
    gt_file_name = file_name.split('.')[0] + '.txt'
    gt_file_path = Path(f'data/{dataset}/explanation/{gt_file_name}') # TODO
    if gt_file_path.exists():
        return get_txt_content_as_str(gt_file_path)
    
    gt_file_name = file_name + '.txt'
    gt_file_path = Path(f'data/{dataset}/explanation/{gt_file_name}') # TODO
    if gt_file_path.exists():
        return get_txt_content_as_str(gt_file_path)


    vuln_detail_csv_path = f'data/{dataset}/original/vuln_detail.csv'
    if os.path.exists(vuln_detail_csv_path):
        df = pd.read_csv(vuln_detail_csv_path)
        for index, row in df.iterrows():
            candidate = row['file_name']
            if file_name == candidate:
                gt = row['explanation']
                return gt

    raise ValueError('gt description not found: ', dataset, file_name)



'''
audit report (audit_results/{auditor}/{method}.csv)
file_name,audit_report,input_token_num,output_token_num,inference_time

eval report (evaluate/{eval_mode}/{auditor}/{method}.csv)
file_name,audit_report, gt, eval_score, eval_input_token_num, eval_output_token_num, eval_inference_time
'''
def evaluate_audit_result_single_csv(evaluator: Evaluator, dataset, audit_result_path, eval_path):

    if os.path.exists(eval_path):
        logger.info(f'eval result exists: {eval_path}')
        # check whether eval_score != -1
        # redo the -1 item
        eval_df = pd.read_csv(eval_path)
        audit_df = pd.read_csv(audit_result_path)
        for index, row in eval_df.iterrows():
            if row['eval_score'] not in [1,2,3,4] or type(row['audit_report']) is not str or len(row['audit_report']) == 0 or row['audit_report'] == 'Inference failed':
                logger.info(f'redo the -1 item: {eval_path}, {row["file_name"]}')
                # update audit_report
                predicted_report = audit_df.at[index, 'audit_report']
                eval_df.at[index, 'audit_report'] = predicted_report
                if type(predicted_report) is not str or len(predicted_report) == 0 or predicted_report == 'Inference failed':
                    logger.error(f'predicted_report is not str: {predicted_report}, index: {index} path: {audit_result_path}')
                    eval_score = -1
                    runtime_info = {
                        'input_token_num': 0,
                        'output_token_num': 0,
                        'inference_time': 0
                    }
                else:
                    correct_report = row['gt']
                    eval_score, runtime_info = evaluator.evaluate(predicted_report=predicted_report, correct_report=correct_report)
                # update eval_score and runtime_info
                eval_df.at[index, f'eval_score'] = eval_score
                for info in runtime_info:
                    eval_df.at[index, f'eval_{info}'] = runtime_info[info]
        eval_df.to_csv(eval_path, index=False)
        logger.info(f'\neval result saved to: {eval_path}')
        return

    logger.info(f'evaluating: {audit_result_path}')
    try:
        audit_df = pd.read_csv(audit_result_path)
    except Exception as e:
        logger.error(f'failed to read audit result csv: {audit_result_path}, error: {e}')
        exit(1)
    eval_df = audit_df[['file_name', 'audit_report']].copy()
    
    for index, row in audit_df.iterrows():
        file_name = row['file_name']
        case_vuln_type = ' '.join(file_name.split('.')[0].split('_')[1:])
        vuln_explan = index_gt_description_as_str(dataset, file_name)
    
        correct_report = None    
        if dataset.startswith('test'):
            correct_report = case_vuln_type
        elif dataset.startswith('smartbugs'):
            correct_report = case_vuln_type
        elif dataset.startswith('ReposVul_cpp'):
            correct_report = vuln_explan
        elif dataset.startswith('ReposVul_py'):
            correct_report = vuln_explan
        elif dataset.startswith('PrimeVul_c'):
            correct_report = vuln_explan

        if correct_report is None:
            logger.error(f'correct report not found for {dataset}, {file_name}')
            raise ValueError(f'correct report not found for {dataset}, {file_name}')

        predicted_report = row['audit_report']
        if type(predicted_report) is not str or len(predicted_report) == 0 or predicted_report == 'Inference failed':
            logger.error(f'predicted_report is not str: {predicted_report}, index: {index} path: {audit_result_path}')
            eval_score = -1
            runtime_info = {
                'input_token_num': 0,
                'output_token_num': 0,
                'inference_time': 0
            }
        else:
            eval_score, runtime_info = evaluator.evaluate(predicted_report=predicted_report, correct_report=correct_report)
        eval_df.at[index, f'gt'] = correct_report
        eval_df.at[index, f'eval_score'] = eval_score
        for info in runtime_info:
            eval_df.at[index, f'eval_{info}'] = runtime_info[info]
        #print('.', end='', flush=True)
        log_dot(logger)
    #print('\n')

    eval_df.to_csv(eval_path, index=False)
    logger.info(f'\neval result saved to: {eval_path}')


'''
- working_dir
-- audit_result
    -- Mixtral
        -- xx.csv
    -- Phi
    -- ...
-- evaluate
-- ..
'''
def batch_eval(dataset, auditor_names):
    evaluator = init_evaluator()
    audit_dir = Path(f'results') / dataset / 'audit_results'
    evaluate_dir = Path(f'results') / dataset / 'evaluate'

    if 'all' in auditor_names:
        auditor_names = LLM_AUDITORS
    if 'local' in auditor_names:
        auditor_names = LOCAL_AUDITORS
    if 'api' in auditor_names:
        auditor_names = API_AUDITORS
    for auditor in auditor_names:
        audit_result_dir = audit_dir / auditor
        if audit_result_dir.exists():
            for file in audit_result_dir.iterdir():
                audit_result_csv_path = audit_dir / auditor / file.name
                eval_csv_path = evaluate_dir / auditor / file.name
                eval_csv_path.parent.mkdir(parents=True, exist_ok=True)
                evaluate_audit_result_single_csv(
                    evaluator=evaluator, 
                    dataset=dataset,
                    audit_result_path=audit_result_csv_path, 
                    eval_path=eval_csv_path)
        else:
            raise ValueError('audit result not found: ', audit_result_dir)
    logger.info('finished')


def batch_eval_parallel(dataset, auditor_names, max_workers=4):
    audit_dir = Path('results') / dataset / 'audit_results'
    evaluate_dir = Path('results') / dataset / 'evaluate'

    if 'all' in auditor_names:
        auditor_names = LLM_AUDITORS
    if 'local' in auditor_names:
        auditor_names = LOCAL_AUDITORS
    if 'api' in auditor_names:
        auditor_names = API_AUDITORS
    if 'agent' in auditor_names:
        auditor_names = AGENT_AUDITORS


    tasks = []
    for auditor in auditor_names:
        audit_result_dir = audit_dir / auditor
        if not audit_result_dir.exists():
            raise ValueError(f'audit result not found: {audit_result_dir}')

        for file in audit_result_dir.iterdir():
            audit_result_csv_path = audit_result_dir / file.name
            eval_csv_path = evaluate_dir / auditor / file.name
            eval_csv_path.parent.mkdir(parents=True, exist_ok=True)

            # 每个任务携带独立 evaluator
            evaluator = init_evaluator()
            tasks.append((evaluator, dataset, audit_result_csv_path, eval_csv_path))

    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(evaluate_audit_result_single_csv, *task) for task in tasks]
        for future in concurrent.futures.as_completed(futures):
            future.result()

def main(args):
    #batch_eval(args.dataset, args.auditors)
    batch_eval_parallel(args.dataset, args.auditors, max_workers=4)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Evaluator")
    parser.add_argument('--dataset', type=str, required=True, help='Dataset to audit')
    parser.add_argument('--auditors', type=str, nargs='+', choices=['all', 'local', 'api', 'agent'] + LLM_AUDITORS, default=["all"], help='Models to evaluate, e.g., all; specify multiple models by space (default: all)')

    args = parser.parse_args()
 
    times = 5
    while (times > 0):
        main(args)
        times -= 1