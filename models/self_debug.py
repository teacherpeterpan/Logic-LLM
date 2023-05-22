from utils import *

folio_debug_prompt_gpt4 = '''
Task: Given the initial program and the error message, debug the following first-order logic program.
You can use the following logical operations:
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
Format for each fact and query: logic form ::: description
------
Initial Program:
Facts:
Implies(Atom('PerformInSchoolTalentShowsOften', '$x1'), And(Atom('AttendSchoolEvents', '$x1'), Atom('VeryEngagedWithSchoolEvents', '$x1'))) ::: If people perform in school talent shows often, then they attend and are very engaged with school events.
Forall('$x1', Xor(Atom('PerformInSchoolTalentShowsOften', '$x1'), Atom('InactiveAndDisinterested', '$x1'))) ::: People either perform in school talent shows often or are inactive and disinterested members of their community.
Implies(Atom('ChaperoneHighSchoolDances', '$x1'), Not(Atom('Student', '$x1'))) ::: If people chaperone high school dances, then they are not students who attend the school.
Forall('$x1', Implies(Atom('InactiveAndDisinterested', '$x1'), Atom('ChaperoneHighSchoolDances', '$x1'))) ::: All people who are inactive and disinterested members of their community chaperone high school dances.
Forall('$x1', Implies(And(Atom('YoungChildOrTeenager', '$x1'), Atom('WishToFurtherAcademicCareer', '$x1')), Atom('Student', '$x1'))) ::: All young children and teenagers who wish to further their academic careers and educational opportunities are students who attend the school.
Xor(AndList([Atom('AttendSchoolEvents', 'bonnie'), Atom('VeryEngagedWithSchoolEvents', 'bonnie'), Atom('Student', 'bonnie')]), AndList([Not(Atom('AttendSchoolEvents', 'bonnie')), Not(Atom('VeryEngagedWithSchoolEvents', 'bonnie')), Not(Atom('Student', 'bonnie'))])) ::: Bonnie either both attends and is very engaged with school events and is a student who attends the school, or she neither attends and is very engaged with school events nor is a student who attends the school.
Query:
Atom('PerformInSchoolTalentShowsOften', 'bonnie') ::: Bonnie performs in school talent shows often.
Error Message:
Can't modify database with a query with free variables: Implies(PerformInSchoolTalentShowsOften($x1),And(AttendSchoolEvents($x1),VeryEngagedWithSchoolEvents($x1)))
Corrected Program:
Facts:
Forall('$x1', Implies(Atom('PerformInSchoolTalentShowsOften', '$x1'), And(Atom('AttendSchoolEvents', '$x1'), Atom('VeryEngagedWithSchoolEvents', '$x1')))) ::: If people perform in school talent shows often, then they attend and are very engaged with school events.
Forall('$x1', Xor(Atom('PerformInSchoolTalentShowsOften', '$x1'), Atom('InactiveAndDisinterested', '$x1'))) ::: People either perform in school talent shows often or are inactive and disinterested members of their community.
Forall('$x1', Implies(Atom('ChaperoneHighSchoolDances', '$x1'), Not(Atom('Student', '$x1')))) ::: If people chaperone high school dances, then they are not students who attend the school.
Forall('$x1', Implies(Atom('InactiveAndDisinterested', '$x1'), Atom('ChaperoneHighSchoolDances', '$x1'))) ::: All people who are inactive and disinterested members of their community chaperone high school dances.
Forall('$x1', Implies(And(Atom('YoungChildOrTeenager', '$x1'), Atom('WishToFurtherAcademicCareer', '$x1')), Atom('Student', '$x1'))) ::: All young children and teenagers who wish to further their academic careers and educational opportunities are students who attend the school.
Xor(AndList([Atom('AttendSchoolEvents', 'bonnie'), Atom('VeryEngagedWithSchoolEvents', 'bonnie'), Atom('Student', 'bonnie')]), AndList([Not(Atom('AttendSchoolEvents', 'bonnie')), Not(Atom('VeryEngagedWithSchoolEvents', 'bonnie')), Not(Atom('Student', 'bonnie'))])) ::: Bonnie either both attends and is very engaged with school events and is a student who attends the school, or she neither attends and is very engaged with school events nor is a student who attends the school.
Query:
Atom('PerformInSchoolTalentShowsOften', 'bonnie') ::: Bonnie performs in school talent shows often.
------
Initial Program:
Facts:
Atom('Capital', 'beijing', 'peoples_republic_of_china') ::: Beijing is the capital of the People's Republic of China.
Atom('MostPopulousNationalCapitalCity', 'beijing') ::: Beijing is the world's most populous national capital city.
Atom('LocatedIn', 'beijing', 'northern_china') ::: Beijing is located in Northern China.
AndList([Atom('Host', 'beijing', '2008_summer_olympics'), Atom('Host', 'beijing', '2008_summer_paralympics')]) ::: Beijing hosted the 2008 Summer Olympics and 2008 Summer Paralympics Games.
AndList([Atom('Host', 'beijing', 'summer_olympics'), Atom('Host', 'beijing', 'winter_olympics'), Atom('Host', 'beijing', 'summer_paralympics'), Atom('Host', 'beijing', 'winter_paralympics')]) ::: Beijing has hosted both the Summer and Winter Olympics, along with the Summer and Winter Paralympics.
Forall('$x1', Implies(Atom('University', '$x1', 'beijing'), Atom('RankAmongBest', '$x1', 'asia_pacific_and_world'))) ::: Many of Beijing's 91 universities consistently rank among the best in the Asia-Pacific and the world.
Query:
And(Atom('Host', 'beijing', '2008_summer_olympics'), Atom('Host', 'beijing', 'winter_olympics')) ::: Beijing has hosted both the 2008 Summer Olympics and a winter olympics.
Error Message:
Constants must start with a lowercase letter, but got 2008_summer_olympics
Corrected Program:
Facts:
Atom('Capital', 'beijing', 'peoples_republic_of_china') ::: Beijing is the capital of the People's Republic of China.
Atom('MostPopulousNationalCapitalCity', 'beijing') ::: Beijing is the world's most populous national capital city.
Atom('LocatedIn', 'beijing', 'northern_china') ::: Beijing is located in Northern China.
AndList([Atom('Host', 'beijing', 'placeholder_2008_summer_olympics'), Atom('Host', 'beijing', 'placeholder_2008_summer_paralympics')]) ::: Beijing hosted the 2008 Summer Olympics and 2008 Summer Paralympics Games.
AndList([Atom('Host', 'beijing', 'summer_olympics'), Atom('Host', 'beijing', 'winter_olympics'), Atom('Host', 'beijing', 'summer_paralympics'), Atom('Host', 'beijing', 'winter_paralympics')]) ::: Beijing has hosted both the Summer and Winter Olympics, along with the Summer and Winter Paralympics.
Forall('$x1', Implies(Atom('University', '$x1', 'beijing'), Atom('RankAmongBest', '$x1', 'asia_pacific_and_world'))) ::: Many of Beijing's 91 universities consistently rank among the best in the Asia-Pacific and the world.
Query:
And(Atom('Host', 'beijing', 'placeholder_2008_summer_olympics'), Atom('Host', 'beijing', 'winter_olympics')) ::: Beijing has hosted both the 2008 Summer Olympics and a winter olympics.
------
Initial Program:
Facts:
Forall('$x1', Implies(Atom('InternationalStudentInUS', '$x1'), Xor(Atom('F1Visa', '$x1'), Atom('J1Visa', '$x1')))) ::: International students in US have either an F1 visa or a J1 visa.
Forall('$x1', Implies(And(Atom('InternationalStudentInUS', '$x1'), Atom('F1Visa', '$x1'), Atom('WantsToWorkInUS', '$x1')), Or(Atom('ApplyForCPT', '$x1'), Atom('ApplyForOPT', '$x1')))) ::: An international student in US with an F1 visa needs to apply for CPT or OPT if the student wants to work in the US.
AndList([Atom('InternationalStudentInUS', 'mike'), Implies(Atom('WantsToWorkInUS', 'mike'), Atom('ApplyForCPT', 'mike'))]) ::: Mike is an international student. Mike needs to apply for CPT if he wants to work in the US.
Query:
Atom('F1Visa', 'mike') ::: Mike has an F1 visa.
Error Message:
And.__init__() takes 3 positional arguments but 4 were given
Corrected Program:
Facts:
Forall('$x1', Implies(Atom('InternationalStudentInUS', '$x1'), Xor(Atom('F1Visa', '$x1'), Atom('J1Visa', '$x1')))) ::: International students in US have either an F1 visa or a J1 visa.
Forall('$x1', Implies(AndList([Atom('InternationalStudentInUS', '$x1'), Atom('F1Visa', '$x1'), Atom('WantsToWorkInUS', '$x1')]), Or(Atom('ApplyForCPT', '$x1'), Atom('ApplyForOPT', '$x1')))) ::: An international student in US with an F1 visa needs to apply for CPT or OPT if the student wants to work in the US.
AndList([Atom('InternationalStudentInUS', 'mike'), Implies(Atom('WantsToWorkInUS', 'mike'), Atom('ApplyForCPT', 'mike'))]) ::: Mike is an international student. Mike needs to apply for CPT if he wants to work in the US.
Query:
Atom('F1Visa', 'mike') ::: Mike has an F1 visa.
------
Initial Program:
%s
Error Message:
%s
Corrected Program:'''

folio_debug_prompt_gpt3 = '''
Task: Given the initial program and the error message, debug the following first-order logic program.
You can use the following logical operations:
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
Format for each fact and query: logic form ::: description
------
Initial Program:
Facts:
Implies(Atom('PerformInSchoolTalentShowsOften', '$x1'), And(Atom('AttendSchoolEvents', '$x1'), Atom('VeryEngagedWithSchoolEvents', '$x1'))) ::: If people perform in school talent shows often, then they attend and are very engaged with school events.
Forall('$x1', Xor(Atom('PerformInSchoolTalentShowsOften', '$x1'), Atom('InactiveAndDisinterested', '$x1'))) ::: People either perform in school talent shows often or are inactive and disinterested members of their community.
Implies(Atom('ChaperoneHighSchoolDances', '$x1'), Not(Atom('Student', '$x1'))) ::: If people chaperone high school dances, then they are not students who attend the school.
Forall('$x1', Implies(Atom('InactiveAndDisinterested', '$x1'), Atom('ChaperoneHighSchoolDances', '$x1'))) ::: All people who are inactive and disinterested members of their community chaperone high school dances.
Forall('$x1', Implies(And(Atom('YoungChildOrTeenager', '$x1'), Atom('WishToFurtherAcademicCareer', '$x1')), Atom('Student', '$x1'))) ::: All young children and teenagers who wish to further their academic careers and educational opportunities are students who attend the school.
Query:
Atom('PerformInSchoolTalentShowsOften', 'bonnie') ::: Bonnie performs in school talent shows often.
Error Message:
Can't modify database with a query with free variables: Implies(PerformInSchoolTalentShowsOften($x1),And(AttendSchoolEvents($x1),VeryEngagedWithSchoolEvents($x1)))
Corrected Program:
Facts:
Forall('$x1', Implies(Atom('PerformInSchoolTalentShowsOften', '$x1'), And(Atom('AttendSchoolEvents', '$x1'), Atom('VeryEngagedWithSchoolEvents', '$x1')))) ::: If people perform in school talent shows often, then they attend and are very engaged with school events.
Forall('$x1', Xor(Atom('PerformInSchoolTalentShowsOften', '$x1'), Atom('InactiveAndDisinterested', '$x1'))) ::: People either perform in school talent shows often or are inactive and disinterested members of their community.
Forall('$x1', Implies(Atom('ChaperoneHighSchoolDances', '$x1'), Not(Atom('Student', '$x1')))) ::: If people chaperone high school dances, then they are not students who attend the school.
Forall('$x1', Implies(Atom('InactiveAndDisinterested', '$x1'), Atom('ChaperoneHighSchoolDances', '$x1'))) ::: All people who are inactive and disinterested members of their community chaperone high school dances.
Forall('$x1', Implies(And(Atom('YoungChildOrTeenager', '$x1'), Atom('WishToFurtherAcademicCareer', '$x1')), Atom('Student', '$x1'))) ::: All young children and teenagers who wish to further their academic careers and educational opportunities are students who attend the school.
Query:
Atom('PerformInSchoolTalentShowsOften', 'bonnie') ::: Bonnie performs in school talent shows often.
------
Initial Program:
Facts:
Atom('Capital', 'beijing', 'peoples_republic_of_china') ::: Beijing is the capital of the People's Republic of China.
Atom('MostPopulousNationalCapitalCity', 'beijing') ::: Beijing is the world's most populous national capital city.
Atom('LocatedIn', 'beijing', 'northern_china') ::: Beijing is located in Northern China.
AndList([Atom('Host', 'beijing', '2008_summer_olympics'), Atom('Host', 'beijing', '2008_summer_paralympics')]) ::: Beijing hosted the 2008 Summer Olympics and 2008 Summer Paralympics Games.
Forall('$x1', Implies(Atom('University', '$x1', 'beijing'), Atom('RankAmongBest', '$x1', 'asia_pacific_and_world'))) ::: Many of Beijing's 91 universities consistently rank among the best in the Asia-Pacific and the world.
Query:
And(Atom('Host', 'beijing', '2008_summer_olympics'), Atom('Host', 'beijing', 'winter_olympics')) ::: Beijing has hosted both the 2008 Summer Olympics and a winter olympics.
Error Message:
Constants must start with a lowercase letter, but got 2008_summer_olympics
Corrected Program:
Facts:
Atom('Capital', 'beijing', 'peoples_republic_of_china') ::: Beijing is the capital of the People's Republic of China.
Atom('MostPopulousNationalCapitalCity', 'beijing') ::: Beijing is the world's most populous national capital city.
Atom('LocatedIn', 'beijing', 'northern_china') ::: Beijing is located in Northern China.
AndList([Atom('Host', 'beijing', 'placeholder_2008_summer_olympics'), Atom('Host', 'beijing', 'placeholder_2008_summer_paralympics')]) ::: Beijing hosted the 2008 Summer Olympics and 2008 Summer Paralympics Games.
Forall('$x1', Implies(Atom('University', '$x1', 'beijing'), Atom('RankAmongBest', '$x1', 'asia_pacific_and_world'))) ::: Many of Beijing's 91 universities consistently rank among the best in the Asia-Pacific and the world.
Query:
And(Atom('Host', 'beijing', 'placeholder_2008_summer_olympics'), Atom('Host', 'beijing', 'winter_olympics')) ::: Beijing has hosted both the 2008 Summer Olympics and a winter olympics.
------
Initial Program:
Facts:
Forall('$x1', Implies(Atom('InternationalStudentInUS', '$x1'), Xor(Atom('F1Visa', '$x1'), Atom('J1Visa', '$x1')))) ::: International students in US have either an F1 visa or a J1 visa.
Forall('$x1', Implies(And(Atom('InternationalStudentInUS', '$x1'), Atom('F1Visa', '$x1'), Atom('WantsToWorkInUS', '$x1')), Or(Atom('ApplyForCPT', '$x1'), Atom('ApplyForOPT', '$x1')))) ::: An international student in US with an F1 visa needs to apply for CPT or OPT if the student wants to work in the US.
AndList([Atom('InternationalStudentInUS', 'mike'), Implies(Atom('WantsToWorkInUS', 'mike'), Atom('ApplyForCPT', 'mike'))]) ::: Mike is an international student. Mike needs to apply for CPT if he wants to work in the US.
Query:
Atom('F1Visa', 'mike') ::: Mike has an F1 visa.
Error Message:
And.__init__() takes 3 positional arguments but 4 were given
Corrected Program:
Facts:
Forall('$x1', Implies(Atom('InternationalStudentInUS', '$x1'), Xor(Atom('F1Visa', '$x1'), Atom('J1Visa', '$x1')))) ::: International students in US have either an F1 visa or a J1 visa.
Forall('$x1', Implies(AndList([Atom('InternationalStudentInUS', '$x1'), Atom('F1Visa', '$x1'), Atom('WantsToWorkInUS', '$x1')]), Or(Atom('ApplyForCPT', '$x1'), Atom('ApplyForOPT', '$x1')))) ::: An international student in US with an F1 visa needs to apply for CPT or OPT if the student wants to work in the US.
AndList([Atom('InternationalStudentInUS', 'mike'), Implies(Atom('WantsToWorkInUS', 'mike'), Atom('ApplyForCPT', 'mike'))]) ::: Mike is an international student. Mike needs to apply for CPT if he wants to work in the US.
Query:
Atom('F1Visa', 'mike') ::: Mike has an F1 visa.
------
Initial Program:
%s
Error Message:
%s
Corrected Program:'''

class SelfDebugger:
    def __init__(self, model_name, interpreter, api_key, max_new_tokens, stop_words):
        self.model_name = model_name
        self.interpreter = interpreter
        self.openai_api = OpenAIModel(api_key, model_name, stop_words, max_new_tokens)
        if interpreter == 'fol':
            self.debug_prompt = folio_debug_prompt_gpt4 if model_name == 'gpt-4' else folio_debug_prompt_gpt3
        else:
            raise Exception(f'Error: Self-debug for {interpreter} not supported yet.')

    def debug(self, initial_program, error_message):
        full_debug_prompt = self.debug_prompt % (initial_program, error_message)
        try:
            output = self.openai_api.generate(full_debug_prompt)
        except:
            print('Error: OpenAI API call failed.')
            output = initial_program
        return output