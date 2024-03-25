"""Generates reasoning graphs for a dataset using the baseline LLM model.

Loads the dataset and in-context examples. Creates prompts by filling the in-context example with the dataset sample's context, question, and options. Generates the reasoning graph using the LLM. Extracts the predicted answer. Saves the outputs as JSON containing the predicted reasoning graph and answer.

Can generate sequentially or in batches. Batch generation may fail, in which case it falls back to sequential generation.
"""
import json
import os
from tqdm import tqdm
from collections import OrderedDict
from typing import Dict, List, Tuple
from utils import OpenAIModel, HuggingFaceModel, LLMClass
import argparse

class Model_Baseline:
    def __init__(self, args, llm_model=None):
        self.args = args
        self.data_path = args.data_path
        self.dataset_name = args.dataset_name
        self.split = args.split
        self.model_name = args.model_name
        self.save_path = args.save_path
        self.demonstration_path = args.demonstration_path
        self.mode = args.mode
        self.framework_to_use = args.framework_to_use
        if llm_model is None:
            if self.framework_to_use == "OpenAI":
                self.llm_model = OpenAIModel(args.api_key, args.model_name, args.stop_words, args.max_new_tokens)
            elif self.framework_to_use == "HuggingFace":
                self.llm_model = HuggingFaceModel(model_id=self.model_name, stop_words = args.stop_words, max_new_tokens=args.max_new_tokens, is_AWQ=args.is_AWQ)
            else:
                self.llm_model = LLMClass()
        else:
            self.llm_model = llm_model

        self.model_name=self.model_name.replace("/","-")

        self.prompt_creator = self.prompt_LSAT
        self.label_phrase = 'The correct option is:'
    
    def prompt_LSAT(self, in_context_example, test_example):
        full_prompt = in_context_example
        context = test_example['context'].strip()
        question = test_example['question'].strip()
        options = '\n'.join([opt.strip() for opt in test_example['options']])
        full_prompt = full_prompt.replace('[[CONTEXT]]', context)
        full_prompt = full_prompt.replace('[[QUESTION]]', question)
        full_prompt = full_prompt.replace('[[OPTIONS]]', options)
        return full_prompt

    def load_in_context_examples(self):
        with open(os.path.join(self.demonstration_path, f'{self.dataset_name}_{self.mode}.txt')) as f:
            in_context_examples = f.read()
        return in_context_examples

    def load_raw_dataset(self, split):
        with open(os.path.join(self.data_path, self.dataset_name, f'{split}.json')) as f:
            raw_dataset = json.load(f)
        return raw_dataset

    def reasoning_graph_generation(self):
        # load raw dataset
        raw_dataset = self.load_raw_dataset(self.split)
        print(f"Loaded {len(raw_dataset)} examples from {self.split} split.")

        # load in-context examples
        in_context_examples = self.load_in_context_examples()
        
        outputs = []
        for example in tqdm(raw_dataset):
            question = example['question']

            # create prompt
            full_prompt = self.prompt_creator(in_context_examples, example)
            output = self.llm_model.generate(full_prompt)
            
            # get the answer
            label_phrase = self.label_phrase
            generated_answer = output.split(label_phrase)[-1].strip()
            generated_reasoning = output.split(label_phrase)[0].strip()

            # create output
            output = {'id': example['id'], 
                      'question': question, 
                      'answer': example['answer'], 
                      'predicted_reasoning': generated_reasoning,
                      'predicted_answer': generated_answer,
                      'flag': self.mode}
            outputs.append(output)

        # save outputs        
        with open(os.path.join(self.save_path, f'{self.mode}_{self.dataset_name}_{self.split}_{self.model_name}.json'), 'w') as f:
            json.dump(outputs, f, indent=2, ensure_ascii=False)

    def batch_reasoning_graph_generation(self, batch_size=10):
        # load raw dataset
        raw_dataset = self.load_raw_dataset(self.split)
        print(f"Loaded {len(raw_dataset)} examples from {self.split} split.")

        # load in-context examples
        in_context_examples = self.load_in_context_examples()

        outputs = []
        # split dataset into chunks
        dataset_chunks = [raw_dataset[i:i + batch_size] for i in range(0, len(raw_dataset), batch_size)]
        for chunk in tqdm(dataset_chunks):
            # create prompt
            full_prompts = [self.prompt_creator(in_context_examples, example) for example in chunk]
            try:
                batch_outputs = self.llm_model.batch_generate(full_prompts)
                # create output
                for sample, output in zip(chunk, batch_outputs):
                    # get the answer
                    dict_output = self.update_answer(sample, output)
                    outputs.append(dict_output)
            except:
                # generate one by one if batch generation fails
                for sample, full_prompt in zip(chunk, full_prompts):
                    try:
                        output = self.llm_model.generate(full_prompt)
                        # get the answer
                        dict_output = self.update_answer(sample, output)
                        outputs.append(dict_output)
                    except:
                        print('Error in generating example: ', sample['id'])

        # save outputs        
        with open(os.path.join(self.save_path, f'{self.mode}_{self.dataset_name}_{self.split}_{self.model_name}.json'), 'w') as f:
            json.dump(outputs, f, indent=2, ensure_ascii=False)
    
    def update_answer(self, sample, output):
        label_phrase = self.label_phrase
        generated_answer = output.split(label_phrase)[-1].strip()
        generated_reasoning = output.split(label_phrase)[0].strip()
        dict_output = {'id': sample['id'], 
                        'question': sample['question'], 
                        'answer': sample['answer'], 
                        'predicted_reasoning': generated_reasoning,
                        'predicted_answer': generated_answer,
                        'flag': self.mode
                        }
        return dict_output

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--data_path', type=str, default='./data')
    parser.add_argument('--dataset_name', type=str)
    parser.add_argument('--split', type=str)
    parser.add_argument('--save_path', type=str, default='./outputs/results')
    parser.add_argument('--demonstration_path', type=str, default='./models/prompts')
    parser.add_argument('--api_key', type=str, default='KEY')
    parser.add_argument('--model_name', type=str)
    parser.add_argument('--framework_to_use', type=str, default='HuggingFace')
    parser.add_argument('--stop_words', type=str, default='------')
    parser.add_argument('--mode', type=str)
    parser.add_argument('--max_new_tokens', type=int, default=1024)
    parser.add_argument('--is_AWQ', type=str, default="auto")
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = parse_args()
    model_problem_reduction = Model_Baseline(args)
    model_problem_reduction.batch_reasoning_graph_generation(batch_size=10)
