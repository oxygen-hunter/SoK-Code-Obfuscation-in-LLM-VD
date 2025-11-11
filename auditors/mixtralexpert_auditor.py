import os
from auditors.local_auditor import LocalLLMAuditor

class MixtralExpertAuditor(LocalLLMAuditor):
    def __init__(self, max_new_tokens, **kwargs):
        super().__init__(
            use_rag=False,
            max_new_tokens=max_new_tokens,
            support_system_prompt=True,
            linebreak='<0x0A>',
            model_id = "mistralai/Mixtral-8x7B-Instruct-v0.1",
            local_model_dir = os.path.expanduser('~/.cache/huggingface/hub/models--mistralai--Mixtral-8x7B-Instruct-v0.1/snapshots/67c3bf6b0aa8b6670f892629331d314f9af0b522'),
            **kwargs
        )