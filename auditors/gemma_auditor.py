import os
from auditors.local_auditor import LocalLLMAuditor

class GemmaAuditor(LocalLLMAuditor):
    def __init__(self, max_new_tokens, **kwargs):
        super().__init__(
            use_rag=False,
            max_new_tokens=max_new_tokens,
            support_system_prompt=False,
            linebreak='\n',
            model_id = "google/gemma-2-2b-it",
            local_model_dir = os.path.expanduser('~/.cache/huggingface/hub/models--google--gemma-2-2b-it/snapshots/299a8560bedf22ed1c72a8a11e7dce4a7f9f51f8'),
            **kwargs
        )