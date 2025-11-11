import os
from auditors.local_auditor import LocalLLMAuditor

class PhiAuditor(LocalLLMAuditor):
    def __init__(self, max_new_tokens, use_flash_attn, **kwargs):
        super().__init__(
            use_rag=False,
            max_new_tokens=max_new_tokens,
            use_flash_attn=use_flash_attn,
            support_system_prompt=True,
            linebreak='<0x0A>',
            model_id = "microsoft/Phi-3-medium-4k-instruct",
            local_model_dir = os.path.expanduser('~/.cache/huggingface/hub/models--microsoft--Phi-3-medium-4k-instruct/snapshots/ae004ae82eb6eddc32906dfacb1d6dfea8f91996'),
            **kwargs
        )