import os
from auditors.local_auditor import LocalLLMAuditor

class Qwen_2p5_14B_Instruct_Auditor(LocalLLMAuditor):
    def __init__(self, max_new_tokens, **kwargs):
        super().__init__(
            use_rag=False,
            max_new_tokens=max_new_tokens,
            support_system_prompt=True,
            linebreak='<0x0A>', # untested
            model_id = "Qwen/Qwen2.5-14B-Instruct",
            local_model_dir = os.path.expanduser('~/models/Qwen2.5-14B-Instruct'),
            **kwargs
        )

class Qwen_2p5_32B_Instruct_Auditor(LocalLLMAuditor):
    def __init__(self, max_new_tokens, **kwargs):
        super().__init__(
            use_rag=False,
            max_new_tokens=max_new_tokens,
            support_system_prompt=True,
            linebreak='<0x0A>', # untested
            model_id = "Qwen/Qwen2.5-32B-Instruct",
            local_model_dir = os.path.expanduser('~/models/Qwen2.5-32B-Instruct'),
            **kwargs
        )