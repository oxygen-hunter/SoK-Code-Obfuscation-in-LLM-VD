from auditors.base_auditor import BaseAuditor
from openai import OpenAI
import httpx
from utils.logger import logger
import time

class OpenRouterAPIAuditor(BaseAuditor):

    def __init__(
            self, 
            model: str, 
            api_key: str, 
            proxy: str = None, 
            max_new_tokens:int = None, 
            temperature :int = 0
        ):
        self.api_key = api_key
        self.model = model
        self.client = OpenAI(
            api_key=self.api_key,
            base_url="https://openrouter.ai/api/v1",
            http_client=httpx.Client(proxy=proxy) if proxy else None
        )
        self.max_new_tokens = max_new_tokens
        self.temperature = temperature
        logger.info(f"LLMAuditor initialized with model: {self.model}")

    def inference(self, system_prompt, user_prompt):
        start_time = time.time()
        messages = [
            {"role": "system", "content": f"{system_prompt}"},
            {"role": "user", "content": f"{user_prompt}"},
        ]

        try:
            response = self.client.chat.completions.create(
                model=self.model,
                max_tokens=self.max_new_tokens,
                temperature=self.temperature,
                messages=messages,
                # extra_body={"include_reasoning": False}
            )

            if len(response.choices[0].message.content) == 0:
                time.sleep(12)
                response = self.client.chat.completions.create(
                    model=self.model,
                    max_tokens=self.max_new_tokens,
                    temperature=self.temperature,
                    messages=messages,
                )

            inference_time = time.time() - start_time

            return (
                response.choices[0].message.content,
                response.usage.prompt_tokens,
                response.usage.completion_tokens,
                inference_time
            )
        except Exception as e:
            logger.info(f"First inference attempt failed: {e}. Retrying after 10s...")
            time.sleep(10)
            try:
                response = self.client.chat.completions.create(
                    model=self.model,
                    max_tokens=self.max_new_tokens,
                    temperature=self.temperature,
                    messages=messages,
                )

                inference_time = time.time() - start_time

                return (
                    response.choices[0].message.content,
                    response.usage.prompt_tokens,
                    response.usage.completion_tokens,
                    inference_time
                )
            except Exception as e:
                logger.error(f"Second inference attempt failed: {e}")
                inference_time = time.time() - start_time
                return (
                    'Inference failed',
                    0,
                    0,
                    inference_time
                )

        