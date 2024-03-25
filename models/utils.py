"""
Utility functions for working with large language models.

Includes functions for generating text from models like GPT-3, handling OpenAI API requests, and timing out long-running requests.

The main classes are:

- HuggingFaceModel: Generates text using a HuggingFace model.
- OpenAIModel: Generates text using an OpenAI model. 
- LLMClass: Abstract base class for LLM models.
- Timeout: Context manager for timing out functions.

The OpenAIModel and HuggingFaceModel classes handle generating text with different underlying models.
The Timeout context manager allows limiting execution time.
"""
import backoff  # for exponential backoff
import openai
import os
import asyncio
from typing import Any

@backoff.on_exception(backoff.expo, openai.error.RateLimitError)
def completions_with_backoff(**kwargs):
    return openai.Completion.create(**kwargs)

@backoff.on_exception(backoff.expo, openai.error.RateLimitError)
def chat_completions_with_backoff(**kwargs):
    return openai.ChatCompletion.create(**kwargs)

async def dispatch_openai_chat_requests(
    messages_list: list[list[dict[str,Any]]],
    model: str,
    temperature: float,
    max_tokens: int,
    top_p: float,
    stop_words: list[str]
) -> list[str]:
    """Dispatches requests to OpenAI API asynchronously.
    
    Args:
        messages_list: List of messages to be sent to OpenAI ChatCompletion API.
        model: OpenAI model to use.
        temperature: Temperature to use for the model.
        max_tokens: Maximum number of tokens to generate.
        top_p: Top p to use for the model.
        stop_words: List of words to stop the model from generating.
    Returns:
        List of responses from OpenAI API.
    """
    async_responses = [
        openai.ChatCompletion.acreate(
            model=model,
            messages=x,
            temperature=temperature,
            max_tokens=max_tokens,
            top_p=top_p,
            stop = stop_words
        )
        for x in messages_list
    ]
    return await asyncio.gather(*async_responses)

async def dispatch_openai_prompt_requests(
    messages_list: list[list[dict[str,Any]]],
    model: str,
    temperature: float,
    max_tokens: int,
    top_p: float,
    stop_words: list[str]
) -> list[str]:
    async_responses = [
        openai.Completion.acreate(
            model=model,
            prompt=x,
            temperature=temperature,
            max_tokens=max_tokens,
            top_p=top_p,
            frequency_penalty = 0.0,
            presence_penalty = 0.0,
            stop = stop_words
        )
        for x in messages_list
    ]
    return await asyncio.gather(*async_responses)

from signal import signal, alarm, SIGALRM
import time

class TimeoutError(Exception):
    ...

class Timeout:
    def __init__(self, seconds=1, message="Timed out"):
        self._seconds = seconds
        self._message = message

    @property
    def seconds(self):
        return self._seconds

    @property
    def message(self):
        return self._message
    
    @property
    def handler(self):
        return self._handler

    @handler.setter
    def handler(self, handler):
        self._handler = handler

    def handle_timeout(self, *_):
        raise TimeoutError(self.message)

    def __enter__(self):
        self.handler = signal(SIGALRM, self.handle_timeout)
        alarm(self.seconds)
        return self

    def __exit__(self, *_):
        alarm(0)
        signal(SIGALRM, self.handler)



from typing import List

class LLMClass:
    def generate(self, input_string: str, temperature: float = 0.0) -> str:
        return input_string

    def batch_generate(self, messages_list: List[str], temperature: float = 0.0) -> List[str]:
        return messages_list


from transformers import pipeline
import torch
from transformers import AutoTokenizer, AutoModelForCausalLM
from transformers import StoppingCriteria, StoppingCriteriaList
from awq import AutoAWQForCausalLM

class StoppingCriteriaToken(StoppingCriteria):

    def __init__(self, stops = []):
        super().__init__()
        self.stops = stops

    def __call__(self, input_ids: torch.LongTensor, scores: torch.FloatTensor):
        stop_count = 0
        for stop in self.stops:
            if stop == int(input_ids[0][-1]):
                return True
                
        return False

class HuggingFaceModel(LLMClass):
    def __init__(self, model_id, stop_words, max_new_tokens, is_AWQ, timeout_time=300, batch_size=10) -> None:
        self.model_id = model_id
        self.tokenizer = AutoTokenizer.from_pretrained(model_id)
        self.timeout_time = timeout_time
        
        if is_AWQ == "auto":
            if "AWQ" in model_id:
                is_AWQ = True
            else:
                is_AWQ = False
        else:
            is_AWQ = bool(is_AWQ)

        if is_AWQ:
            model = AutoAWQForCausalLM.from_quantized(model_id, fuse_layers=True, device_map = 'auto',
                                          trust_remote_code=False, safetensors=True)
            self.model = model.model
        else:
            self.model = AutoModelForCausalLM.from_pretrained(model_id, device_map="auto", torch_dtype="auto")

        stop_token_ids = [self.tokenizer.convert_tokens_to_ids(stop_token) for stop_token in stop_words.split(" ")]
        stopping_criteria = StoppingCriteriaList([StoppingCriteriaToken(stops=stop_token_ids)])

        self.pipe = pipeline("text-generation", model=self.model, tokenizer=self.tokenizer, max_new_tokens=max_new_tokens, batch_size=batch_size, device_map="auto", do_sample=False, top_p = 1.0, return_full_text=False, stopping_criteria = stopping_criteria)
        if self.pipe.tokenizer.pad_token_id is None:
            self.pipe.tokenizer.pad_token_id = self.pipe.model.config.eos_token_id

    def generate(self, input_string, temperature = 0.0):
        with Timeout(self.timeout_time): # time out after 5 minutes
            try:
                response = self.pipe(input_string, temperature=temperature)
                generated_text = response[0]["generated_text"].strip()
                return generated_text
            except TimeoutError as e:
                print(e)
                print(input_string)
                return 'Time out!'

    def batch_generate(self, messages_list, temperature = 0.0):
        with Timeout(self.timeout_time): # time out after 5 minutes
            try:
                responses = self.pipe(messages_list, temperature=temperature)
                generated_text = [response[0]["generated_text"].strip() for response in responses]
                return generated_text
            except TimeoutError as e:
                print(e)
                print(messages_list)
                return ['Time out!' for m in messages_list]


class OpenAIModel(LLMClass):
    def __init__(self, API_KEY, model_name, stop_words, max_new_tokens) -> None:
        openai.api_key = API_KEY
        self.model_name = model_name
        self.max_new_tokens = max_new_tokens
        self.stop_words = stop_words

    # used for chat-gpt and gpt-4
    def chat_generate(self, input_string, temperature = 0.0):
        response = chat_completions_with_backoff(
                model = self.model_name,
                messages=[
                        {"role": "user", "content": input_string}
                    ],
                max_tokens = self.max_new_tokens,
                temperature = temperature,
                top_p = 1.0,
                stop = self.stop_words
        )
        generated_text = response['choices'][0]['message']['content'].strip()
        return generated_text
    
    # used for text/code-davinci
    def prompt_generate(self, input_string, temperature = 0.0):
        response = completions_with_backoff(
            model = self.model_name,
            prompt = input_string,
            max_tokens = self.max_new_tokens,
            temperature = temperature,
            top_p = 1.0,
            frequency_penalty = 0.0,
            presence_penalty = 0.0,
            stop = self.stop_words
        )
        generated_text = response['choices'][0]['text'].strip()
        return generated_text

    def generate(self, input_string, temperature = 0.0):
        if self.model_name in ['text-davinci-002', 'code-davinci-002', 'text-davinci-003']:
            return self.prompt_generate(input_string, temperature)
        elif self.model_name in ['gpt-4', 'gpt-3.5-turbo']:
            return self.chat_generate(input_string, temperature)
        else:
            raise Exception("Model name not recognized")
    
    def batch_chat_generate(self, messages_list, temperature = 0.0):
        open_ai_messages_list = []
        for message in messages_list:
            open_ai_messages_list.append(
                [{"role": "user", "content": message}]
            )
        predictions = asyncio.run(
            dispatch_openai_chat_requests(
                    open_ai_messages_list, self.model_name, temperature, self.max_new_tokens, 1.0, self.stop_words
            )
        )
        return [x['choices'][0]['message']['content'].strip() for x in predictions]
    
    def batch_prompt_generate(self, prompt_list, temperature = 0.0):
        predictions = asyncio.run(
            dispatch_openai_prompt_requests(
                    prompt_list, self.model_name, temperature, self.max_new_tokens, 1.0, self.stop_words
            )
        )
        return [x['choices'][0]['text'].strip() for x in predictions]

    def batch_generate(self, messages_list, temperature = 0.0):
        if self.model_name in ['text-davinci-002', 'code-davinci-002', 'text-davinci-003']:
            return self.batch_prompt_generate(messages_list, temperature)
        elif self.model_name in ['gpt-4', 'gpt-3.5-turbo']:
            return self.batch_chat_generate(messages_list, temperature)
        else:
            raise Exception("Model name not recognized")

    def generate_insertion(self, input_string, suffix, temperature = 0.0):
        response = completions_with_backoff(
            model = self.model_name,
            prompt = input_string,
            suffix= suffix,
            temperature = temperature,
            max_tokens = self.max_new_tokens,
            top_p = 1.0,
            frequency_penalty = 0.0,
            presence_penalty = 0.0
        )
        generated_text = response['choices'][0]['text'].strip()
        return generated_text