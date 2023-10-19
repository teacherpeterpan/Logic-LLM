from nltk.inference.prover9 import *

# set the path to the prover9 executable
os.environ['PROVER9'] = '../models/symbolic_solvers/Prover9/bin'

arguments = [
    ("(man(x) <-> (not (not man(x))))", []),
    ("(not (man(x) & (not man(x))))", []),
    ("(man(x) | (not man(x)))", []),
    ("(man(x) & (not man(x)))", []),
    ("(man(x) -> man(x))", []),
    ("(not (man(x) & (not man(x))))", []),
    ("(man(x) | (not man(x)))", []),
    ("(man(x) -> man(x))", []),
    ("(man(x) <-> man(x))", []),
    ("(not (man(x) <-> (not man(x))))", []),
    ("mortal(Socrates)", ["all x.(man(x) -> mortal(x))", "man(Socrates)"]),
    ("((all x.(man(x) -> walks(x)) & man(Socrates)) -> some y.walks(y))", []),
    ("(all x.man(x) -> all x.man(x))", []),
    ("some x.all y.sees(x,y)", []),
    (
        "some e3.(walk(e3) & subj(e3, mary))",
        [
            "some e1.(see(e1) & subj(e1, john) & some e2.(pred(e1, e2) & walk(e2) & subj(e2, mary)))"
        ],
    ),
    (
        "some x e1.(see(e1) & subj(e1, x) & some e2.(pred(e1, e2) & walk(e2) & subj(e2, mary)))",
        [
            "some e1.(see(e1) & subj(e1, john) & some e2.(pred(e1, e2) & walk(e2) & subj(e2, mary)))"
        ],
    ),
]

expressions = [
    r"some x y.sees(x,y)",
    r"some x.(man(x) & walks(x))",
    r"\x.(man(x) & walks(x))",
    r"\x y.sees(x,y)",
    r"walks(john)",
    r"\x.big(x, \y.mouse(y))",
    r"(walks(x) & (runs(x) & (threes(x) & fours(x))))",
    r"(walks(x) -> runs(x))",
    r"some x.(PRO(x) & sees(John, x))",
    r"some x.(man(x) & (not walks(x)))",
    r"all x.(man(x) -> walks(x))",
]


def spacer(num=45):
    print("-" * num)


def test_config():
    a = Expression.fromstring("(walk(j) & sing(j))")
    g = Expression.fromstring("walk(j)")
    p = Prover9Command(g, assumptions=[a])
    p._executable_path = None
    p.prover9_search = []
    p.prove()
    # config_prover9('/usr/local/bin')
    print(p.prove())
    print(p.proof())



def test_convert_to_prover9(expr):
    """
    Test that parsing works OK.
    """
    for t in expr:
        e = Expression.fromstring(t)
        print(convert_to_prover9(e))


def test_prove(arguments):
    """
    Try some proofs and exhibit the results.
    """
    for (goal, assumptions) in arguments:
        g = Expression.fromstring(goal)
        alist = [Expression.fromstring(a) for a in assumptions]
        p = Prover9Command(g, assumptions=alist).prove()
        for a in alist:
            print("   %s" % a)
        print(f"|- {g}: {p}\n")


def demo():
    print("Testing configuration")
    spacer()
    test_config()
    print()
    print("Testing conversion to Prover9 format")
    spacer()
    test_convert_to_prover9(expressions)
    print()
    print("Testing proofs")
    spacer()
    test_prove(arguments)


if __name__ == "__main__":
    demo()