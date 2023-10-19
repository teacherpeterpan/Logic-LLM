import json
import random
import os

class Backup_Answer_Generator:
    def __init__(self, dataset_name, backup_strategy, backup_LLM_result_path) -> None:
        self.dataset_name = dataset_name
        self.backup_strategy = backup_strategy
        self.backup_LLM_result_path = backup_LLM_result_path
        if self.backup_strategy == 'LLM':
            with open(backup_LLM_result_path, 'r') as f:
                LLM_result = json.load(f)
            self.backup_results = {sample['id'] : sample['predicted_answer'] for sample in LLM_result}

    def get_backup_answer(self, id):
        if self.backup_strategy == 'random':
            return self.random_backup()
        elif self.backup_strategy == 'LLM':
            return self.LLM_backup(id)
        
    def random_backup(self):
        if self.dataset_name == 'ProntoQA':
            return random.choice(['A', 'B'])
        elif self.dataset_name == 'ProofWriter' or self.dataset_name == 'FOLIO':
            return random.choice(['A', 'B', 'C'])
        elif self.dataset_name == 'AR-LSAT':
            return random.choice(['A', 'B', 'C', 'D', 'E'])
        else:
            raise ValueError(f'Invalid dataset name: {self.dataset_name}')
        
    def LLM_backup(self, id):
        return self.backup_results[id]