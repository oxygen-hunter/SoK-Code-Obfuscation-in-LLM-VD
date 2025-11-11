import time
from auditors.base_auditor import BaseAuditor
from transformers import AutoTokenizer, AutoModelForCausalLM, BitsAndBytesConfig
import torch
from transformers import pipeline
from utils.logger import logger

class LocalLLMAuditor(BaseAuditor):

    def __init__(
        self, 
        use_rag = False, 
        max_new_tokens = None, 
        temperature = 0,
        use_flash_attn=False,
        support_system_prompt=False, 
        linebreak = None,
        model_id = None, 
        local_model_dir = None
    ):
        super().__init__(use_rag)
        logger.info(f"model {model_id} initializing")

        self.max_new_tokens = max_new_tokens
        self.temperature = temperature
        self.support_system_prompt = support_system_prompt
        self.linebreak = linebreak

        quantization_config = BitsAndBytesConfig(
            load_in_4bit=True,
            bnb_4bit_use_double_quant=True,
            bnb_4bit_quant_type="nf4",
            bnb_4bit_compute_dtype=torch.bfloat16,
        )

        # 模型路径优先使用本地，否则用model_id
        model_path = local_model_dir if local_model_dir is not None else model_id

        # GPU memory
        max_memory = {
            #0: "23GiB",
            1: "23GiB",
            2: "23GiB",
            3: "23GiB",
        }

        # 通用参数
        model_kwargs = {
            "quantization_config": quantization_config,
            "device_map": "auto",
            "max_memory": max_memory,
        }

        # Flash Attention 相关参数（如果启用）
        if use_flash_attn:
            model_kwargs.update({
                "torch_dtype": "auto",
                "trust_remote_code": True,
                "attn_implementation": "flash_attention_2",
            })

        self.model = AutoModelForCausalLM.from_pretrained(
            model_path,
            **model_kwargs
        )
        self.tokenizer = AutoTokenizer.from_pretrained(model_path)

        logger.info(f"model {model_id} initialed successfully")
    

    def inference(self, system_prompt, user_prompt):
        start_time = time.time()
        
        if self.support_system_prompt:
            messages = [
                {"role": "system", "content": f"{system_prompt}"},
                {"role": "user", "content": f"{user_prompt}"},
            ]
        else:
            messages = [
                {"role": "user", "content": f"{system_prompt + '\n' + user_prompt}"},
            ]
        
        pipe = pipeline(
            "text-generation",
            model=self.model,
            tokenizer=self.tokenizer,
        )

        generation_args = {
            "max_new_tokens": self.max_new_tokens,
            "return_full_text": False,
            "temperature": self.temperature,
            # "do_sample": False,
            "pad_token_id": pipe.tokenizer.eos_token_id
        }

        outputs = pipe(messages, **generation_args)

        inference_time = time.time() - start_time

        results = outputs[0]['generated_text']

        input_ids = self.tokenizer(user_prompt, return_tensors="pt").to("cuda")
        input_token_length = input_ids["input_ids"].shape[1]
        output_ids = self.tokenizer(results, return_tensors="pt").to("cuda")
        output_token_length = output_ids["input_ids"].shape[1]
        return results, input_token_length, output_token_length, inference_time