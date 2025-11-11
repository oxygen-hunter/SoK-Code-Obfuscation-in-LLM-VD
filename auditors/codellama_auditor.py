import os
from auditors.local_auditor import LocalLLMAuditor

class CodeLlamaAuditor(LocalLLMAuditor):
    def __init__(self, max_new_tokens, **kwargs):
        super().__init__(
            use_rag=False,
            max_new_tokens=max_new_tokens,
            support_system_prompt=True,
            linebreak='<0x0A>',
            model_id = "codellama/CodeLlama-7b-Instruct-hf",
            local_model_dir = os.path.expanduser('~/.cache/huggingface/hub/models--codellama--CodeLlama-7b-Instruct-hf/snapshots/22cb240e0292b0b5ab4c17ccd97aa3a2f799cbed'),
            **kwargs
        )