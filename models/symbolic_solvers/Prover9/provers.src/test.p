cnf(a, b, 'hi there!' | 'hello " there!').

cnf(wolf_is_an_animal,axiom,
    ( animal(X)
    | ~ wolf(X) )).

cnf(fox_is_an_animal,axiom,
    ( animal(X)
    | ~ fox(X) )).

cnf(bird_is_an_animal,axiom,
    ( animal(X)
    | ~ bird(X) )).

