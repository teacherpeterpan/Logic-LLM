API_KEY=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
DATASET=ProofWriter
SPLIT=dev
# MODEL=text-davinci-003
MODEL=gpt-4
MODE=CoT

python gpt3_baseline.py \
    --api_key $API_KEY \
    --model_name $MODEL \
    --dataset_name $DATASET \
    --split $SPLIT \
    --mode $MODE \
    --max_new_tokens 1024 \