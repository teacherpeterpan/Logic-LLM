import os
import time
from utils import OpenAIModel, HuggingFaceModel, LLMClass
import model_baseline
import logic_inference
import logic_program
import self_refinement
import argparse
import evaluation
import traceback
import torch

# to prevent reinitialization of LLM multiple times

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--data_path', type=str, default='./data')
    parser.add_argument('--dataset_name', type=str, default="AR-LSAT")
    parser.add_argument('--split', type=str, default='dev')
    parser.add_argument('--save_path', type=str, default='./outputs/logic_inference')
    parser.add_argument('--demonstration_path', type=str, default='./models/prompts')
    parser.add_argument('--backup_strategy', type=str, default='LLM', choices=['random', 'LLM'])
    parser.add_argument('--backup_LLM_result_path', type=str, default='./outputs/results')
    parser.add_argument('--model_name', type=str, default='text-davinci-003')
    parser.add_argument('--framework_to_use', type=str, default='HuggingFace')
    parser.add_argument('--timeout', type=int, default=60)
    parser.add_argument('--api_key', type=str, default='KEY')
    parser.add_argument('--stop_words', type=str, default='------')
    parser.add_argument('--max_new_tokens', type=int, default=1024)
    parser.add_argument('--is_AWQ', type=str, default="auto")
    parser.add_argument('--mode', type=str, default='CoT')
    parser.add_argument('--refiment', type=int, default=0)
    parser.add_argument('--maximum_rounds', type=int, default=3)
    parser.add_argument('--batch_size', type=int, default=10)
    parser.add_argument('--timeout_time', type=int, default=300)
    parser.add_argument('--result_path', type=str, default='./outputs/logic_inference')
    args, unknown = parser.parse_known_args()
    return args

if __name__ == "__main__":

    overall_start = time.time()

    args = parse_args()
    framework_to_use = args.framework_to_use

    if framework_to_use == "OpenAI":
        llm_model = OpenAIModel(args.api_key, 'gpt-4', args.stop_words, args.max_new_tokens)
    elif framework_to_use == "HuggingFace":
        llm_model = HuggingFaceModel(model_id=args.model_name, stop_words = args.stop_words, max_new_tokens=args.max_new_tokens, is_AWQ=args.is_AWQ, timeout_time=args.timeout_time, batch_size=args.batch_size)
    else:
        llm_model = LLMClass()

    for dataset in ["ProntoQA" , "ProofWriter" , "FOLIO" , "LogicalDeduction" , "AR-LSAT"]:
        torch.cuda.empty_cache()

        dataset_start = time.time()

        try:
            print("Dataset: ", dataset)
            args.dataset_name = dataset

            print("CoT Solution")
            model_problem_reduction = model_baseline.Model_Baseline(args, llm_model)
            model_problem_reduction.save_path = args.backup_LLM_result_path
            model_problem_reduction.batch_reasoning_graph_generation(batch_size=args.batch_size)
            print("Direct Solution")
            model_problem_reduction.mode = "Direct"
            model_problem_reduction.batch_reasoning_graph_generation(batch_size=args.batch_size)

            torch.cuda.empty_cache()
            print("Generating Logic Programs")
            logic_program_generator = logic_program.LogicProgramGenerator(args, llm_model)
            logic_program_generator.save_path = './outputs/logic_programs'
            logic_program_generator.batch_logic_program_generation(batch_size=args.batch_size)

            torch.cuda.empty_cache()
            print("Running Logic Programs")
            logic_engine = logic_inference.LogicInferenceEngine(args)
            logic_engine.inference_on_dataset()

            self_refinement_engine = self_refinement.SelfRefinementEngine(args, 1, llm_model)
            for r in range(1, args.maximum_rounds+1):
                print(f"Round {r} self-refinement")
                self_refinement_engine.single_round_self_refinement(batch_size=args.batch_size)
                self_refinement_engine.current_round += 1
                logic_engine.refiment = r
                logic_engine.inference_on_dataset()


            result_file = os.path.join(args.backup_LLM_result_path, f'CoT_{args.dataset_name}_{args.split}_{args.model_name.replace("/","-")}.json')
            print(f'Evaluating {result_file}')
            evaluation.full_evaluation(result_file)


            result_file = os.path.join(args.backup_LLM_result_path, f'Direct_{args.dataset_name}_{args.split}_{args.model_name.replace("/","-")}.json')
            print(f'Evaluating {result_file}')
            evaluation.full_evaluation(result_file)


            result_file = os.path.join(args.result_path, f'{args.dataset_name}_{args.split}_{args.model_name.replace("/","-")}_backup-{args.backup_strategy}.json')
            print(f'Evaluating {result_file}')
            evaluation.full_evaluation(result_file)

            for r in range(1, args.maximum_rounds+1):
                result_file = os.path.join(args.result_path, f'self-refine-{r}_{args.dataset_name}_{args.split}_{args.model_name.replace("/","-")}_backup-{args.backup_strategy}.json')
                print(f'Evaluating {result_file}')
                evaluation.full_evaluation(result_file)

            print(f"Finished {dataset} in {time.time() - dataset_start:.2f} secs")

        except Exception as e:
            print(e)
            print(traceback.format_exc())
            continue

    print(f"Total time: {time.time() - overall_start:.2f} secs")
 

