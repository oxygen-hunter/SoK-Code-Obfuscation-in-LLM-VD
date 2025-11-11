import os
from auditors.local_auditor import LocalLLMAuditor

class MistralAuditor(LocalLLMAuditor):
    def __init__(self, max_new_tokens, **kwargs):
        super().__init__(
            use_rag=False,
            max_new_tokens=max_new_tokens,
            support_system_prompt=True,
            linebreak='<0x0A>',
            model_id = "mistralai/Mistral-7B-Instruct-v0.2",
            local_model_dir = os.path.expanduser('~/.cache/huggingface/hub/models--mistralai--Mistral-7B-Instruct-v0.2/snapshots/3ad372fc79158a2148299e3318516c786aeded6c'),
            **kwargs
        )