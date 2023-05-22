few_shot_protoqa_prompt = """
Task Description: You are given a problem description and a question. The task is to: 
1) define all the predicates in the problem
2) parse the problem into logic rules based on the defined predicates
3) write all the facts mentioned in the problem
4) parse the question into the logic form

Problem:
Each jompus is fruity. Every jompus is a wumpus. Every wumpus is not transparent. Wumpuses are tumpuses. Tumpuses are mean. Tumpuses are vumpuses. Every vumpus is cold. Each vumpus is a yumpus. Yumpuses are orange. Yumpuses are numpuses. Numpuses are dull. Each numpus is a dumpus. Every dumpus is not shy. Impuses are shy. Dumpuses are rompuses. Each rompus is liquid. Rompuses are zumpuses. Alex is a tumpus.
Question:
True or false: Alex is not shy.
Predicates:
Jompus($x, bool) ::: Does x belong to Jompus?
Fruity($x, bool) ::: Is x fruity?
Wumpus($x, bool) ::: Does x belong to Wumpus?
Transparent($x, bool) ::: Is x transparent?
Tumpuses($x, bool) ::: Does x belong to Tumpuses?
Mean($x, bool) ::: Is x mean?
Vumpuses($x, bool) ::: Does x belong to Vumpuses?
Cold($x, bool) ::: Is x cold?
Yumpus($x, bool) ::: Does x belong to Yumpus?
Orange($x, bool) ::: Is x orange?
Numpus($x, bool) ::: Does x belong to Numpus?
Dull($x, bool) ::: Is x dull?
Dumpus($x, bool) ::: Does x belong to Dumpus?
Shy($x, bool) ::: Is x shy?
Impuses($x, bool) ::: Does x belong to Impuses?
Rompus($x, bool) ::: Does x belong to Rompus?
Liquid($x, bool) ::: Is x liquid?
Zumpus($x, bool) ::: Does x belong to Zumpus?
Facts:
Tumpuses(Alex, True)
Rules:
Jompus($x, True) >>> Fruity($x, True)
Jompus($x, True) >>> Wumpus($x, True)
Wumpus($x, True) >>> Transparent($x, False)
Wumpuses($x, True) >>> Tumpuses($x, True)
Tumpuses($x, True) >>> Mean($x, True)
Tumpuses($x, True) >>> Vumpuses($x, True)
Vumpuses($x, True) >>> Cold($x, True)
Vumpuses($x, True) >>> Yumpus($x, True)
Yumpus($x, True) >>> Orange($x, True)
Yumpus($x, True) >>> Numpus($x, True)
Numpus($x, True) >>> Dull($x, True)
Numpus($x, True) >>> Dumpus($x, True)
Dumpus($x, True) >>> Shy($x, False)
Impuses($x, True) >>> Shy($x, True)
Dumpus($x, True) >>> Rompus($x, True)
Rompus($x, True) >>> Liquid($x, True)
Rompus($x, True) >>> Zumpus($x, True)
Query: 
Shy(Alex, False)
------
Problem:
%s
Question:
%s
"""

few_shot_proofwriter_prompt = """
Task Description: You are given a problem description and a question. The task is to: 
1) define all the predicates in the problem
2) parse the problem into logic rules based on the defined predicates
3) write all the facts mentioned in the problem
4) parse the question into the logic form

Problem:
Anne is quiet. Erin is furry. Erin is green. Fiona is furry. Fiona is quiet. Fiona is red. Fiona is rough. Fiona is white. Harry is furry. Harry is quiet. Harry is white. Young people are furry. If Anne is quiet then Anne is red. Young, green people are rough. If someone is green then they are white. If someone is furry and quiet then they are white. If someone is young and white then they are rough. All red people are young.
Question:
Based on the above information, is the following statement true, false, or unknown? Anne is white.
Predicates:
Quiet($x, bool) ::: Is x quiet?
Furry($x, bool) ::: Is x furry?
Green($x, bool) ::: Is x green?
Red($x, bool) ::: Is x red?
Rough($x, bool) ::: Is x rough?
White($x, bool) ::: Is x white?
Young($x, bool) ::: Is x young?
Facts:
Quite(Anne, True) ::: Anne is quiet.
Furry(Erin, True) ::: Erin is furry.
Green(Erin, True) ::: Erin is green.
Furry(Fiona, True) ::: Fiona is furry.
Quite(Fiona, True) ::: Fiona is quiet.
Red(Fiona, True) ::: Fiona is red.
Rough(Fiona, True) ::: Fiona is rough.
White(Fiona, True) ::: Fiona is white.
Furry(Harry, True) ::: Harry is furry.
Quite(Harry, True) ::: Harry is quiet.
White(Harry, True) ::: Harry is white.
Rules:
Young($x, True) >>> Furry($x, True) ::: Young people are furry.
Quite(Anne, True) >>> Red($x, True) ::: If Anne is quiet then Anne is red.
Young($x, True) >>> Rough($x, True) ::: Young, green people are rough.
Green($x, True) >>> Rough($x, True) ::: Young, green people are rough.
Green($x, True) >>> White($x, True) ::: If someone is green then they are white.
Furry($x, True) && Quite($x, True) >>> White($x, True) ::: If someone is furry and quiet then they are white.
Young($x, True) && White($x, True) >>> Rough($x, True) ::: If someone is young and white then they are rough.
Red($x, True) >>> Young($x, True) ::: All red people are young.
Query:
White(Anne, True) ::: Anne is white.
------
Problem:
%s
Question:
%s
"""

few_shot_folio_prompt_fol = '''
Task Description: You are given a problem description and a question. The task is to parse the problem into first-order logic facts and rules and parse the question into the logic form.
We define the following logical operations: 
1) And(arg1, arg2) - logical conjunction of arg1 and arg2
2) Or(arg1, arg2) - logical disjunction of arg1 and arg2
3) Implies(arg1, arg2) - logical implication of arg1 and arg2
4) Not(arg1) - logical negation of arg1
5) Xor(arg1, arg2) - logical exclusive disjunction of arg1 and arg2
6) Forall(variable, arg1) - logical universal quantification of arg1 with respect to variable
7) Exists(variable, arg1) - logical existential quantification of arg1 with respect to variable
8) Atom(predicate, arg1, arg2, ...) - logical atom with predicate and arguments
9) AndList([arg1, arg2, ...]) - logical conjunction of a list of arguments
10) OrList([arg1, arg2, ...]) - logical disjunction of a list of arguments
Output format for each fact: logic form ::: description

Problem:
All people who regularly drink coffee are dependent on caffeine. People either regularly drink coffee or joke about being addicted to caffeine. No one who jokes about being addicted to caffeine is unaware that caffeine is a drug. Rina is either a student and unaware that caffeine is a drug, or neither a student nor unaware that caffeine is a drug. If Rina is not a person dependent on caffeine and a student, then Rina is either a person dependent on caffeine and a student, or neither a person dependent on caffeine nor a student.
Question:
Based on the above information, is the following statement true, false, or uncertain? Rina is either a person who jokes about being addicted to caffeine or is unaware that caffeine is a drug.
Based on the above information, is the following statement true, false, or uncertain? Rina is either a person who regularly drinks coffee or a person who is unaware that caffeine is a drug.
Based on the above information, is the following statement true, false, or uncertain? If Rina is either a person who jokes about being addicted to caffeine and a person who is unaware that caffeine is a drug, or neither a person who jokes about being addicted to caffeine nor a person who is unaware that caffeine is a drug, then Rina jokes about being addicted to caffeine and regularly drinks coffee.
>>>>>>
Facts:
Forall('$x1', Implies(Atom('RegularlyDrinkCoffee', '$x1'), Atom('DependentOnCaffeine', '$x1'))) ::: All people who regularly drink coffee are dependent on caffeine.
Forall('$x1', Xor(Atom('RegularlyDrinkCoffee', '$x1'), Atom('JokeAboutBeingAddictedToCaffeine', '$x1'))) ::: People either regularly drink coffee or joke about being addicted to caffeine.
Forall('$x1', Implies(Atom('JokeAboutBeingAddictedToCaffeine', '$x1'), Not(Atom('UnawareThatCaffeineIsADrug', '$x1')))) ::: No one who jokes about being addicted to caffeine is unaware that caffeine is a drug.
Xor(And(Atom('Student', 'rina'), Atom('UnawareThatCaffeineIsADrug', 'rina')), And(Not(Atom('Student', 'rina')), Not(Atom('UnawareThatCaffeineIsADrug', 'rina')))) ::: Rina is either a student and unaware that caffeine is a drug, or neither a student nor unaware that caffeine is a drug.
Implies(Not(And(Atom('DependentOnCaffeine', 'rina'), Atom('Student', 'rina'))), Xor(And(Atom('DependentOnCaffeine', 'rina'), Atom('Student', 'rina')), And(Not(Atom('DependentOnCaffeine', 'rina')), Not(Atom('Student', 'rina'))))) ::: If Rina is not a person dependent on caffeine and a student, then Rina is either a person dependent on caffeine and a student, or neither a person dependent on caffeine nor a student.
Query:
Xor(Atom('JokeAboutBeingAddictedToCaffeine', 'rina'), Atom('UnawareThatCaffeineIsADrug', 'rina')) ::: Rina is either a person who jokes about being addicted to caffeine or is unaware that caffeine is a drug.
Xor(Atom('RegularlyDrinkCoffee', 'rina'), Atom('UnawareThatCaffeineIsADrug', 'rina')) ::: Rina is either a person who regularly drinks coffee or a person who is unaware that caffeine is a drug.
Implies(Xor(And(Atom('JokeAboutBeingAddictedToCaffeine', 'rina'), Atom('UnawareThatCaffeineIsADrug', 'rina')), And(Not(Atom('JokeAboutBeingAddictedToCaffeine', 'rina')), Not(Atom('UnawareThatCaffeineIsADrug', 'rina')))), And(Atom('JokeAboutBeingAddictedToCaffeine', 'rina'), Atom('RegularlyDrinkCoffee', 'rina'))) ::: If Rina is either a person who jokes about being addicted to caffeine and a person who is unaware that caffeine is a drug, or neither a person who jokes about being addicted to caffeine nor a person who is unaware that caffeine is a drug, then Rina jokes about being addicted to caffeine and regularly drinks coffee.
------
Problem:
Miroslav Venhoda was a Czech choral conductor who specialized in the performance of Renaissance and Baroque music. Any choral conductor is a musician. Some musicians love music. Miroslav Venhoda published a book in 1946 called Method of Studying Gregorian Chant.
Question:
Based on the above information, is the following statement true, false, or uncertain? Miroslav Venhoda loved music.
Based on the above information, is the following statement true, false, or uncertain? A Czech person wrote a book in 1946.
Based on the above information, is the following statement true, false, or uncertain? No choral conductor specialized in the performance of Renaissance.
>>>>>>
Facts:
AndList([Atom('ChoralConductor', 'miroslav_venhoda'), Atom('Czech', 'miroslav_venhoda'), Atom('Specialize', 'miroslav_venhoda', 'renaissance'), Atom('Specialize', 'miroslav_venhoda', 'baroque')]) ::: Miroslav Venhoda was a Czech choral conductor who specialized in the performance of Renaissance and Baroque music.
Forall('$x1', Implies(Atom('ChoralConductor', '$x1'), Atom('Musician', '$x1'))) ::: Any choral conductor is a musician.
Exists('$x1', Implies(Atom('Musician', '$x1'), Atom('LoveMusic', '$x1'))) ::: Some musicians love music.
AndList([Atom('Book', 'method_of_studying_gregorian_chant'), Atom('Publish', 'method_of_studying_gregorian_chant', 'year1946'), Atom('Author', 'method_of_studying_gregorian_chant', 'miroslav_venhoda')]) ::: Miroslav Venhoda published a book in 1946 called Method of Studying Gregorian Chant.
Query:
Atom('LoveMusic', 'miroslav_venhoda') ::: Miroslav Venhoda loved music.
Exists('$x2', Exists('$x1', AndList([Atom('Czech', '$x1'), Atom('Author', '$x2', '$x1'), Atom('Book', '$x2'), Atom('Publish', '$x2', 'year1946')]))) ::: A Czech person wrote a book in 1946.
Not(Exists('$x1', And(Atom('ChoralConductor', '$x1'), Atom('Specialize', '$x1', 'renaissance')))) ::: No choral conductor specialized in the performance of Renaissance.
------
Problem:
%s
Question:
%s
>>>>>>
'''

few_shot_logical_deduction_prompt = '''
Task Description: You are given a problem description. The task is to parse the problem as a constraint satisfaction problem, defining the domain, variables, and contraints.

Problem:
The following paragraphs each describe a set of three objects arranged in a fixed order. The statements are logically consistent within each paragraph.\n\nIn an antique car show, there are three vehicles: a station wagon, a convertible, and a minivan. The station wagon is the oldest. The minivan is newer than the convertible.
Question:
Which of the following is true?
Options:
A) The station wagon is the second-newest.
B) The convertible is the second-newest.
C) The minivan is the second-newest.
Domain:
1: oldest
3: newest
Variables:
station_wagon [IN] [1, 2, 3]
convertible [IN] [1, 2, 3]
minivan [IN] [1, 2, 3]
Constraints:
station_wagon == 1 ::: The station wagon is the oldest.
minivan > convertible ::: The minivan is newer than the convertible.
AllDifferentConstraint([station_wagon, convertible, minivan]) ::: All vehicles have different values.
Query:
A) station_wagon == 2 ::: The station wagon is the second-newest.
B) convertible == 2 ::: The convertible is the second-newest.
C) minivan == 2 ::: The minivan is the second-newest.
------
Problem:
The following paragraphs each describe a set of five objects arranged in a fixed order. The statements are logically consistent within each paragraph.\n\nOn a branch, there are five birds: a quail, an owl, a raven, a falcon, and a robin. The owl is the leftmost. The robin is to the left of the raven. The quail is the rightmost. The raven is the third from the left.
Question:
Which of the following is true?
Options:
A) The quail is the rightmost.
B) The owl is the rightmost.
C) The raven is the rightmost.
D) The falcon is the rightmost.
E) The robin is the rightmost.
Domain:
1: leftmost
5: rightmost
Variables:
quail [IN] [1, 2, 3, 4, 5]
owl [IN] [1, 2, 3, 4, 5]
raven [IN] [1, 2, 3, 4, 5]
falcon [IN] [1, 2, 3, 4, 5]
robin [IN] [1, 2, 3, 4, 5]
Constraints:
owl == 1 ::: The owl is the leftmost.
robin < raven ::: The robin is to the left of the raven.
quail == 5 ::: The quail is the rightmost.
raven == 3 ::: The raven is the third from the left.
AllDifferentConstraint([quail, owl, raven, falcon, robin]) ::: All birds have different values.
Query:
A) quail == 5 ::: The quail is the rightmost.
B) owl == 5 ::: The owl is the rightmost.
C) raven == 5 ::: The raven is the rightmost.
D) falcon == 5 ::: The falcon is the rightmost.
E) robin == 5 ::: The robin is the rightmost.
------
Problem:
%s
Question:
%s
Options:
%s
'''