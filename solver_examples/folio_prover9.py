'''
Example problem:
'predicates': ['Movie(x)', 'HappyEnding(x)'],
'clauses': ['¬∀x (Movie(x) → HappyEnding(x))', 'Movie(titanic)', '¬HappyEnding(titanic)', 'Movie(lionKing)', 'HappyEnding(lionKing)'],
'conclusion': '∃x (Movie(x) ∧ ¬HappyEnding(x))',
'label': 'TRUE'
'''
from nltk.inference.prover9 import *

# set the path to the prover9 executable
os.environ['PROVER9'] = '../models/symbolic_solvers/Prover9/bin'

argument = (
    'some x.(Movie(x) & not HappyEnding(x))',
    [
        'not all x.(Movie(x) -> HappyEnding(x))',
        'Movie(titanic)',
        'not HappyEnding(titanic)',
        'Movie(lionKing)',
        'HappyEnding(lionKing)',
    ]
)

def prove(argument):
    goal, assumptions = argument
    g = Expression.fromstring(goal)
    alist = [Expression.fromstring(a) for a in assumptions]
    p = Prover9Command(g, assumptions=alist).prove()
    for a in alist:
        print("   %s" % a)
    print(f"==> {g}: {p}\n")

if __name__ == '__main__':
    prove(argument)
    