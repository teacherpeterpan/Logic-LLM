# Simple logical inference system: resolution and model checking for first-order logic.
# @author Percy Liang

import collections

# Recursively apply str inside map
def rstr(x):
    if isinstance(x, tuple): return str(tuple(map(rstr, x)))
    if isinstance(x, list): return str(list(map(rstr, x)))
    if isinstance(x, set): return str(set(map(rstr, x)))
    if isinstance(x, dict):
        newx = {}
        for k, v in list(x.items()):
            newx[rstr(k)] = rstr(v)
        return str(newx)
    return str(x)

class Expression:
    # Helper functions used by subclasses.
    def ensureType(self, arg, wantedType):
        if not isinstance(arg, wantedType):
            raise Exception('%s: wanted %s, but got %s' % (self.__class__.__name__, wantedType, arg))
        return arg
    def ensureFormula(self, arg): return self.ensureType(arg, Formula)
    def ensureFormulas(self, args):
        for arg in args: self.ensureFormula(arg)
        return args
    def isa(self, wantedType): return isinstance(self, wantedType)
    def join(self, args): return ','.join(str(arg) for arg in args)

    def __eq__(self, other): return str(self) == str(other)
    def __hash__(self): return hash(str(self))
    # Cache the string to be more efficient
    def __repr__(self):
        if not self.strRepn: self.strRepn = self.computeStrRepn()
        return self.strRepn

# A Formula represents a truth value.
class Formula(Expression): pass

# A Term coresponds to an object.
class Term(Expression): pass

# Variable symbol (must start with '$')
# Example: $x
class Variable(Term):
    def __init__(self, name):
        if not name.startswith('$'): raise Exception('Variable must start with "$", but got %s' % name)
        self.name = name
        self.strRepn = None
    def computeStrRepn(self): return self.name

# Constant symbol (must be uncapitalized)
# Example: john
class Constant(Term):
    def __init__(self, name):
        if not name[0].islower(): raise Exception('Constants must start with a lowercase letter, but got %s' % name)
        self.name = name
        self.strRepn = None
    def computeStrRepn(self): return self.name

# Predicate symbol (must be capitalized) applied to arguments.
# Example: LivesIn(john, palo_alto)
class Atom(Formula):
    def __init__(self, name, *args):
        if not name[0].isupper(): raise Exception('Predicates must start with a uppercase letter, but got %s' % name)
        self.name = name
        self.args = list(map(toExpr, args))
        self.strRepn = None
    def computeStrRepn(self):
        if len(self.args) == 0: return self.name
        return self.name + '(' + self.join(self.args) + ')'

def toExpr(x):
    if isinstance(x, str):
        if x.startswith('$'): return Variable(x)
        return Constant(x)
    return x

AtomFalse = False
AtomTrue = True

# Example: Not(Rain)
class Not(Formula):
    def __init__(self, arg):
        self.arg = self.ensureFormula(arg)
        self.strRepn = None
    def computeStrRepn(self): return 'Not(' + str(self.arg) + ')'

# Example: And(Rain,Snow)
class And(Formula):
    def __init__(self, arg1, arg2):
        self.arg1 = self.ensureFormula(arg1)
        self.arg2 = self.ensureFormula(arg2)
        self.strRepn = None
    def computeStrRepn(self): return 'And(' + str(self.arg1) + ',' + str(self.arg2) + ')'

# Example: Or(Rain,Snow)
class Or(Formula):
    def __init__(self, arg1, arg2):
        self.arg1 = self.ensureFormula(arg1)
        self.arg2 = self.ensureFormula(arg2)
        self.strRepn = None
    def computeStrRepn(self): return 'Or(' + str(self.arg1) + ',' + str(self.arg2) + ')'

# Example: Implies(Rain,Wet)
class Implies(Formula):
    def __init__(self, arg1, arg2):
        self.arg1 = self.ensureFormula(arg1)
        self.arg2 = self.ensureFormula(arg2)
        self.strRepn = None
    def computeStrRepn(self): return 'Implies(' + str(self.arg1) + ',' + str(self.arg2) + ')'

# Example: Exists($x,Lives(john, $x))
class Exists(Formula):
    def __init__(self, var, body):
        self.var = self.ensureType(toExpr(var), Variable)
        self.body = self.ensureFormula(body)
        self.strRepn = None
    def computeStrRepn(self): return 'Exists(' + str(self.var) + ',' + str(self.body) + ')'

# Example: Forall($x,Implies(Human($x),Alive($x)))
class Forall(Formula):
    def __init__(self, var, body):
        self.var = self.ensureType(toExpr(var), Variable)
        self.body = self.ensureFormula(body)
        self.strRepn = None
    def computeStrRepn(self): return 'Forall(' + str(self.var) + ',' + str(self.body) + ')'

# Take a list of conjuncts / disjuncts and return a formula
def AndList(forms):
    result = AtomTrue
    for form in forms:
        result = And(result, form) if result != AtomTrue else form
    return result
def OrList(forms):
    result = AtomFalse
    for form in forms:
        result = Or(result, form) if result != AtomFalse else form
    return result

# Return list of conjuncts of |form|.
# Example: And(And(A, Or(B, C)), Not(D)) => [A, Or(B, C), Not(D)]
def flattenAnd(form):
    if form.isa(And): return flattenAnd(form.arg1) + flattenAnd(form.arg2)
    else: return [form]

# Return list of disjuncts of |form|.
# Example: Or(Or(A, And(B, C)), D) => [A, And(B, C), Not(D)]
def flattenOr(form):
    if form.isa(Or): return flattenOr(form.arg1) + flattenOr(form.arg2)
    else: return [form]

# Syntactic sugar
def Equiv(a, b): return And(Implies(a, b), Implies(b, a))
def Xor(a, b): return And(Or(a, b), Not(And(a, b)))

# Special predicate which is used internally (e.g., in propositionalization).
def Equals(x, y): return Atom('Equals', x, y)

# Given a predicate name (e.g., Parent), return a formula that asserts that
# that predicate is anti-reflexive
# (e.g., Not(Parent(x,x))).
def AntiReflexive(predicate):
    #return Forall('$x', Not(Atom(predicate, '$x', '$x')))
    # Force Equals() to be used and show up in the models.
    return Forall('$x', Forall('$y', Implies(Atom(predicate, '$x', '$y'),
                                             Not(Equals('$x', '$y')))))

############################################################
# Simple inference rules

# A Rule takes a sequence of argument Formulas and produces a set of result
# Formulas (possibly [] if the rule doesn't apply).
class Rule:
    pass

class UnaryRule(Rule):
    def applyRule(self, form): raise Exception('Override me')

class BinaryRule(Rule):
    def applyRule(self, form1, form2): raise Exception('Override me')
    # Override if rule is symmetric to save a factor of 2.
    def symmetric(self): return False

############################################################
# Unification

# Mutate |subst| with variable => bindings
# Return whether unification was successful
# Assume forms are in CNF.
# Note: we don't do occurs check because we don't have function symbols.
def unify(form1, form2, subst):
    if form1.isa(Variable): return unifyTerms(form1, form2, subst)
    if form1.isa(Constant): return unifyTerms(form1, form2, subst)
    if form1.isa(Atom):
        return form2.isa(Atom) and form1.name == form2.name and len(form1.args) == len(form2.args) and \
            all(unify(form1.args[i], form2.args[i], subst) for i in range(len(form1.args)))
    if form1.isa(Not):
        return form2.isa(Not) and unify(form1.arg, form2.arg, subst)
    if form1.isa(And):
        return form2.isa(And) and unify(form1.arg1, form2.arg1, subst) and unify(form1.arg2, form2.arg2, subst)
    if form1.isa(Or):
        return form2.isa(Or) and unify(form1.arg1, form2.arg1, subst) and unify(form1.arg2, form2.arg2, subst)
    raise Exception('Unhandled: %s' % form1)

# Follow multiple links to get to x
def getSubst(subst, x):
    while True:
        y = subst.get(x)
        if y == None: return x
        x = y

def unifyTerms(a, b, subst):
    #print 'unifyTerms', a, b, rstr(subst)
    a = getSubst(subst, a)
    b = getSubst(subst, b)
    if a == b: return True
    if a.isa(Variable): subst[a] = b
    elif b.isa(Variable): subst[b] = a
    else: return False
    return True

# Assume form in CNF.
def applySubst(form, subst):
    if len(subst) == 0: return form
    if form.isa(Variable):
        #print 'applySubst', rstr(form), rstr(subst), rstr(subst.get(form, form))
        #return subst.get(form, form)
        return getSubst(subst, form)
    if form.isa(Constant): return form
    if form.isa(Atom): return Atom(*[form.name] + [applySubst(arg, subst) for arg in form.args])
    if form.isa(Not): return Not(applySubst(form.arg, subst))
    if form.isa(And): return And(applySubst(form.arg1, subst), applySubst(form.arg2, subst))
    if form.isa(Or): return Or(applySubst(form.arg1, subst), applySubst(form.arg2, subst))
    raise Exception('Unhandled: %s' % form)

############################################################
# Convert to CNF, Resolution rules

def withoutElementAt(items, i): return items[0:i] + items[i+1:]

def negateFormula(item):
    return item.arg if item.isa(Not) else Not(item)

# Given a list of Formulas, return a new list with:
# - If A and Not(A) exists, return [AtomFalse] for conjunction, [AtomTrue] for disjunction
# - Remove duplicates
# - Sort the list
def reduceFormulas(items, mode):
    for i in range(len(items)):
        for j in range(i+1, len(items)):
            if negateFormula(items[i]) == items[j]:
                if mode == And: return [AtomFalse]
                elif mode == Or: return [AtomTrue]
                else: raise Exception("Invalid mode: %s" % mode)
    items = sorted(set(items), key=str)
    return items

# Generate a list of all subexpressions of a formula (including terms).
# Example:
# - Input: And(Atom('A', Constant('a')), Atom('B'))
# - Output: [And(Atom('A', Constant('a')), Atom('B')), Atom('A', Constant('a')), Constant('a'), Atom('B')]
def allSubexpressions(form):
    subforms = []
    def recurse(form):
        subforms.append(form)
        if form.isa(Variable): pass
        elif form.isa(Constant): pass
        elif form.isa(Atom):
            for arg in form.args: recurse(arg)
        elif form.isa(Not): recurse(form.arg)
        elif form.isa(And): recurse(form.arg1); recurse(form.arg2)
        elif form.isa(Or): recurse(form.arg1); recurse(form.arg2)
        elif form.isa(Implies): recurse(form.arg1); recurse(form.arg2)
        elif form.isa(Exists): recurse(form.body)
        elif form.isa(Forall): recurse(form.body)
        else: raise Exception("Unhandled: %s" % form)
    recurse(form)
    return subforms

# Return a list of the free variables in |form|.
def allFreeVars(form):
    variables = []
    def recurse(form, boundVars):
        if form.isa(Variable):
            if form not in boundVars: variables.append(form)
        elif form.isa(Constant): pass
        elif form.isa(Atom):
            for arg in form.args: recurse(arg, boundVars)
        elif form.isa(Not): recurse(form.arg, boundVars)
        elif form.isa(And): recurse(form.arg1, boundVars); recurse(form.arg2, boundVars)
        elif form.isa(Or): recurse(form.arg1, boundVars); recurse(form.arg2, boundVars)
        elif form.isa(Implies): recurse(form.arg1, boundVars); recurse(form.arg2, boundVars)
        elif form.isa(Exists): recurse(form.body, boundVars + [form.var])
        elif form.isa(Forall): recurse(form.body, boundVars + [form.var])
        else: raise Exception("Unhandled: %s" % form)
    recurse(form, [])
    return variables

# Return |form| with all free occurrences of |var| replaced with |obj|.
def substituteFreeVars(form, var, obj):
    def recurse(form, boundVars):
        if form.isa(Variable):
            if form == var: return obj
            return form
        elif form.isa(Constant): return form
        elif form.isa(Atom):
            return Atom(*[form.name] + [recurse(arg, boundVars) for arg in form.args])
        elif form.isa(Not): return Not(recurse(form.arg, boundVars))
        elif form.isa(And): return And(recurse(form.arg1, boundVars), recurse(form.arg2, boundVars))
        elif form.isa(Or): return Or(recurse(form.arg1, boundVars), recurse(form.arg2, boundVars))
        elif form.isa(Implies): return Implies(recurse(form.arg1, boundVars), recurse(form.arg2, boundVars))
        elif form.isa(Exists):
            if form.var == var: return form  # Don't substitute inside
            return Exists(form.var, recurse(form.body, boundVars + [form.var]))
        elif form.isa(Forall):
            if form.var == var: return form  # Don't substitute inside
            return Forall(form.var, recurse(form.body, boundVars + [form.var]))
        else: raise Exception("Unhandled: %s" % form)
    return recurse(form, [])

def allConstants(form):
    return [x for x in allSubexpressions(form) if x.isa(Constant)]

class ToCNFRule(UnaryRule):
    def __init__(self):
        # For standardizing variables.
        # For each existing variable name, the number of times it has occurred
        self.varCounts = collections.Counter()

    def applyRule(self, form):
        newForm = form

        # Step 1: remove implications
        def removeImplications(form):
            if form.isa(Atom): return form
            if form.isa(Not): return Not(removeImplications(form.arg))
            if form.isa(And): return And(removeImplications(form.arg1), removeImplications(form.arg2))
            if form.isa(Or): return Or(removeImplications(form.arg1), removeImplications(form.arg2))
            if form.isa(Implies): return Or(Not(removeImplications(form.arg1)), removeImplications(form.arg2))
            if form.isa(Exists): return Exists(form.var, removeImplications(form.body))
            if form.isa(Forall): return Forall(form.var, removeImplications(form.body))
            raise Exception("Unhandled: %s" % form)
        newForm = removeImplications(newForm)

        # Step 2: push negation inwards (de Morgan)
        def pushNegationInwards(form):
            if form.isa(Atom): return form
            if form.isa(Not):
                if form.arg.isa(Not):  # Double negation
                    return pushNegationInwards(form.arg.arg)
                if form.arg.isa(And):  # De Morgan
                    return Or(pushNegationInwards(Not(form.arg.arg1)), pushNegationInwards(Not(form.arg.arg2)))
                if form.arg.isa(Or):  # De Morgan
                    return And(pushNegationInwards(Not(form.arg.arg1)), pushNegationInwards(Not(form.arg.arg2)))
                if form.arg.isa(Exists):
                    return Forall(form.arg.var, pushNegationInwards(Not(form.arg.body)))
                if form.arg.isa(Forall):
                    return Exists(form.arg.var, pushNegationInwards(Not(form.arg.body)))
                return form
            if form.isa(And): return And(pushNegationInwards(form.arg1), pushNegationInwards(form.arg2))
            if form.isa(Or): return Or(pushNegationInwards(form.arg1), pushNegationInwards(form.arg2))
            if form.isa(Implies): return Or(Not(pushNegationInwards(form.arg1)), pushNegationInwards(form.arg2))
            if form.isa(Exists): return Exists(form.var, pushNegationInwards(form.body))
            if form.isa(Forall): return Forall(form.var, pushNegationInwards(form.body))
            raise Exception("Unhandled: %s" % form)
        newForm = pushNegationInwards(newForm)

        # Step 3: standardize variables: make sure all variables are different
        # Don't modify subst; return a new version where var is mapped onto
        # something that hasn't been seen before.
        def updateSubst(subst, var):
            self.varCounts[var.name] += 1
            newVar = Variable(var.name + str(self.varCounts[var.name]))
            return dict(list(subst.items()) + [(var, newVar)])
        def standardizeVariables(form, subst):
            if form.isa(Variable):
                if form not in subst: raise Exception("Free variable found: %s" % form)
                return subst[form]
            if form.isa(Constant): return form
            if form.isa(Atom): return Atom(*([form.name] + [standardizeVariables(arg, subst) for arg in form.args]))
            if form.isa(Not): return Not(standardizeVariables(form.arg, subst))
            if form.isa(And): return And(standardizeVariables(form.arg1, subst), standardizeVariables(form.arg2, subst))
            if form.isa(Or): return Or(standardizeVariables(form.arg1, subst), standardizeVariables(form.arg2, subst))
            if form.isa(Exists):
                newSubst = updateSubst(subst, form.var)
                return Exists(newSubst[form.var], standardizeVariables(form.body, newSubst))
            if form.isa(Forall):
                newSubst = updateSubst(subst, form.var)
                return Forall(newSubst[form.var], standardizeVariables(form.body, newSubst))
            raise Exception("Unhandled: %s" % form)
        newForm = standardizeVariables(newForm, {}) 

        # Step 4: replace existentially quantified variables with Skolem functions
        def skolemize(form, subst, scope): 
            if form.isa(Variable): return subst.get(form, form)
            if form.isa(Constant): return form
            if form.isa(Atom): return Atom(*[form.name] + [skolemize(arg, subst, scope) for arg in form.args])
            if form.isa(Not): return Not(skolemize(form.arg, subst, scope))
            if form.isa(And): return And(skolemize(form.arg1, subst, scope), skolemize(form.arg2, subst, scope))
            if form.isa(Or): return Or(skolemize(form.arg1, subst, scope), skolemize(form.arg2, subst, scope))
            if form.isa(Exists):
                # Create a Skolem function that depends on the variables in the scope (list of variables)
                # Example:
                # - Suppose scope = [$x, $y] and form = Exists($z,F($z)).
                # - Normally, we would return F(Z($x,$y)), where Z is a brand new Skolem function.
                # - But since we don't have function symbols, we replace with a Skolem predicate:
                #   Forall($z,Implies(Z($z,$x,$y),F($z)))
                #   Important: when doing resolution, need to catch Not(Z($z,*,*)) as a contradiction.
                if len(scope) == 0:
                    subst[form.var] = Constant('skolem' + form.var.name)
                    return skolemize(form.body, subst, scope)
                else:
                    skolem = Atom(*['Skolem' + form.var.name, form.var] + scope)
                    return Forall(form.var, Or(Not(skolem), skolemize(form.body, subst, scope)))
            if form.isa(Forall):
                return Forall(form.var, skolemize(form.body, subst, scope + [form.var]))
            raise Exception("Unhandled: %s" % form)
        newForm = skolemize(newForm, {}, [])

        # Step 5: remove universal quantifiers [note: need to do this before distribute, unlike Russell/Norvig book]
        def removeUniversalQuantifiers(form):
            if form.isa(Atom): return form
            if form.isa(Not): return Not(removeUniversalQuantifiers(form.arg))
            if form.isa(And): return And(removeUniversalQuantifiers(form.arg1), removeUniversalQuantifiers(form.arg2))
            if form.isa(Or): return Or(removeUniversalQuantifiers(form.arg1), removeUniversalQuantifiers(form.arg2))
            if form.isa(Forall): return removeUniversalQuantifiers(form.body)
            raise Exception("Unhandled: %s" % form)
        newForm = removeUniversalQuantifiers(newForm)

        # Step 6: distribute Or over And (want And on the outside): Or(And(A,B),C) becomes And(Or(A,C),Or(B,C))
        def distribute(form):
            if form.isa(Atom): return form
            if form.isa(Not): return Not(distribute(form.arg))
            if form.isa(And): return And(distribute(form.arg1), distribute(form.arg2))
            if form.isa(Or):
                # First need to distribute as much as possible
                f1 = distribute(form.arg1)
                f2 = distribute(form.arg2)
                if f1.isa(And):
                    return And(distribute(Or(f1.arg1, f2)), distribute(Or(f1.arg2, f2)))
                if f2.isa(And):
                    return And(distribute(Or(f1, f2.arg1)), distribute(Or(f1, f2.arg2)))
                return Or(f1, f2)
            if form.isa(Exists): return Exists(form.var, distribute(form.body))
            if form.isa(Forall): return Forall(form.var, distribute(form.body))
            raise Exception("Unhandled: %s" % form)
        newForm = distribute(newForm)

        # Post-processing: break up conjuncts into conjuncts and sort the disjuncts in each conjunct
        # Remove instances of A and Not(A)
        conjuncts = [OrList(reduceFormulas(flattenOr(f), Or)) for f in flattenAnd(newForm)]
        #print rstr(form), rstr(conjuncts)
        assert len(conjuncts) > 0
        if any(x == AtomFalse for x in conjuncts): return [AtomFalse]
        if all(x == AtomTrue for x in conjuncts): return [AtomTrue]
        conjuncts = [x for x in conjuncts if x != AtomTrue]
        results = reduceFormulas(conjuncts, And)
        if len(results) == 0: results = [AtomFalse]
        #print 'CNF', form, rstr(results)
        return results

class ResolutionRule(BinaryRule):
    # Assume formulas are in CNF
    # Assume A and Not(A) don't both exist in a form (taken care of by CNF conversion)
    def applyRule(self, form1, form2):
        items1 = flattenOr(form1)
        items2 = flattenOr(form2)
        results = []
        #print 'RESOLVE', form1, form2
        for i, item1 in enumerate(items1):
            for j, item2 in enumerate(items2):
                subst = {}
                if unify(negateFormula(item1), item2, subst):
                    newItems1 = withoutElementAt(items1, i)
                    newItems2 = withoutElementAt(items2, j)
                    newItems = [applySubst(item, subst) for item in newItems1 + newItems2]

                    if len(newItems) == 0:  # Contradiction: False
                        results = [AtomFalse]
                        break

                    #print 'STEP: %s %s => %s %s' % (form1, form2, rstr(newItems), rstr(subst))
                    result = OrList(reduceFormulas(newItems, Or))

                    # Not(Skolem$x($x,...)) is a contradiction
                    if isinstance(result, Not) and result.arg.name.startswith('Skolem'):
                        results = [AtomFalse]
                        break

                    # Don't add redundant stuff
                    if result == AtomTrue: continue
                    if result in results: continue

                    results.append(result)
            if results == [AtomFalse]: break

        #print 'RESOLUTION: %s %s => %s' % (form1, form2, rstr(results))
        return results
    def symmetric(self): return True

############################################################
# Model checking

# Return the set of models
def performModelChecking(allForms, findAll, objects=None, verbose=0):
    if verbose >= 3:
        print(('performModelChecking', rstr(allForms)))
    # Propositionalize, convert to CNF, dedup
    allForms = propositionalize(allForms, objects)
    # Convert to CNF: actually makes things slower
    #allForms = [f for form in allForms for f in ToCNFRule().applyRule(form)]
    #if any(x == AtomFalse for x in allForms): return []
    #if all(x == AtomTrue for x in allForms): return [set()]
    #allForms = [x for x in allForms if x != AtomTrue]
    #allForms = reduceFormulas(allForms, And)
    allForms = [universalInterpret(form) for form in allForms]
    allForms = list(set(allForms) - set([AtomTrue, AtomFalse]))
    if verbose >= 3:
        print(('All Forms:', rstr(allForms)))

    if allForms == []: return [set()]  # One model
    if allForms == [AtomFalse]: return []  # No models

    # Atoms are the variables
    atoms = set()
    for form in allForms:
        for f in allSubexpressions(form):
            if f.isa(Atom): atoms.add(f)
    atoms = list(atoms)

    if verbose >= 3:
        print(('Atoms:', rstr(atoms)))
        print(('Constraints:', rstr(allForms)))

    # For each atom, list the set of formulas
    # atom index => list of formulas
    atomForms = [
        (atom, [form for form in allForms if atom in allSubexpressions(form)]) \
        for atom in atoms \
    ]
    # Degree heuristic
    atomForms.sort(key = lambda x : -len(x[1]))
    atoms = [atom for atom, form in atomForms]

    # Keep only the forms for an atom if it only uses atoms up until that point.
    atomPrefixForms = []
    for i, (atom, forms) in enumerate(atomForms):
        prefixForms = []
        for form in forms:
            useAtoms = set(x for x in allSubexpressions(form) if x.isa(Atom))
            if useAtoms <= set(atoms[0:i+1]):
                prefixForms.append(form)
        atomPrefixForms.append((atom, prefixForms))

    if verbose >= 3:
        print('Plan:')
        for atom, forms in atomForms:
            print(("  %s: %s" % (rstr(atom), rstr(forms))))
    assert sum(len(forms) for atom, forms in atomPrefixForms) == len(allForms)

    # Build up an interpretation
    N = len(atoms)
    models = []  # List of models which are true
    model = set()  # Set of true atoms, mutated over time
    def recurse(i): # i: atom index
        if not findAll and len(models) > 0: return
        if i == N:  # Found a model on which the formulas are true
            models.append(set(model))
            return
        atom, forms = atomPrefixForms[i]
        result = universalInterpretAtom(atom)
        if result == None or result == False:
            if interpretForms(forms, model): recurse(i+1)
        if result == None or result == True:
            model.add(atom)
            if interpretForms(forms, model): recurse(i+1)
            model.remove(atom)
    recurse(0)

    if verbose >= 5:
        print('Models:')
        for model in models:
            print(("  %s" % rstr(model)))

    return models

# A model is a set of atoms.
def printModel(model):
    for x in sorted(map(str, model)):
        print(('*', x, '=', 'True'))
    print(('*', '(other atoms if any)', '=', 'False'))

# Convert a first-order logic formula into a propositional formula, assuming
# database semantics.
# Example: Forall becomes And over all objects
# - Input: form = Forall('$x', Atom('Alive', '$x')), objects = ['alice', 'bob']
# - Output: And(Atom('Alive', 'alice'), Atom('Alive', 'bob'))
# Example: Exists becomes Or over all objects
# - Input: form = Exists('$x', Atom('Alive', '$x')), objects = ['alice', 'bob']
# - Output: Or(Atom('Alive', 'alice'), Atom('Alive', 'bob'))
def propositionalize(forms, objects=None):
    # If not specified, set objects to all constants mentioned in in |form|.
    if objects == None:
        objects = set()
        for form in forms:
            objects |= set(allConstants(form))
        objects = list(objects)
    else:
        # Make sure objects are expressions: Convert ['a', 'b'] to [Constant('a'), Constant('b')]
        objects = [toExpr(obj) for obj in objects]

    # Recursively convert |form|, which could contain Exists and Forall, to forms that don't contain these quantifiers.
    # |subst| is a map from variables to constants.
    def convert(form, subst):
        if form.isa(Variable):
            if form not in subst: raise Exception("Free variable found: %s" % form)
            return subst[form]
        if form.isa(Constant): return form
        if form.isa(Atom):
            return Atom(*[form.name] + [convert(arg, subst) for arg in form.args])
        if form.isa(Not): return Not(convert(form.arg, subst))
        if form.isa(And): return And(convert(form.arg1, subst), convert(form.arg2, subst))
        if form.isa(Or): return Or(convert(form.arg1, subst), convert(form.arg2, subst))
        if form.isa(Implies): return Implies(convert(form.arg1, subst), convert(form.arg2, subst))
        if form.isa(Exists):
            return OrList([convert(form.body, dict(list(subst.items()) + [(form.var, obj)])) for obj in objects])
        if form.isa(Forall):
            return AndList([convert(form.body, dict(list(subst.items()) + [(form.var, obj)])) for obj in objects])
        raise Exception("Unhandled: %s" % form)

    # Think of newForms as conjoined
    newForms = []
    # Convert all the forms
    for form in forms:
        newForm = convert(form, {})
        if newForm == AtomFalse: return [AtomFalse]
        if newForm == AtomTrue: continue
        newForms.extend(flattenAnd(newForm))
    return newForms

# Some atoms have a fixed value, so we should just evaluate them.
# Assumption: atom is propositional logic.
def universalInterpretAtom(atom):
    if atom.name == 'Equals':
        return AtomTrue if atom.args[0] == atom.args[1] else AtomFalse
    return None

# Reduce the expression (e.g., Equals(a,a) => True)
# Assumption: atom is propositional logic.
def universalInterpret(form):
    if form.isa(Variable): return form
    if form.isa(Constant): return form
    if form.isa(Atom):
        result = universalInterpretAtom(form)
        if result != None: return result
        return Atom(*[form.name] + [universalInterpret(arg) for arg in form.args])
    if form.isa(Not):
        arg = universalInterpret(form.arg)
        if arg == AtomTrue: return AtomFalse
        if arg == AtomFalse: return AtomTrue
        return Not(arg)
    if form.isa(And):
        arg1 = universalInterpret(form.arg1)
        arg2 = universalInterpret(form.arg2)
        if arg1 == AtomFalse: return AtomFalse
        if arg2 == AtomFalse: return AtomFalse
        if arg1 == AtomTrue: return arg2
        if arg2 == AtomTrue: return arg1
        return And(arg1, arg2)
    if form.isa(Or):
        arg1 = universalInterpret(form.arg1)
        arg2 = universalInterpret(form.arg2)
        if arg1 == AtomTrue: return AtomTrue
        if arg2 == AtomTrue: return AtomTrue
        if arg1 == AtomFalse: return arg2
        if arg2 == AtomFalse: return arg1
        return Or(arg1, arg2)
    if form.isa(Implies):
        arg1 = universalInterpret(form.arg1)
        arg2 = universalInterpret(form.arg2)
        if arg1 == AtomFalse: return AtomTrue
        if arg2 == AtomTrue: return AtomTrue
        if arg1 == AtomTrue: return arg2
        if arg2 == AtomFalse: return Not(arg1)
        return Implies(arg1, arg2)
    raise Exception("Unhandled: %s" % form)

def interpretForm(form, model):
    if form.isa(Atom): return form in model
    if form.isa(Not): return not interpretForm(form.arg, model)
    if form.isa(And): return interpretForm(form.arg1, model) and interpretForm(form.arg2, model)
    if form.isa(Or): return interpretForm(form.arg1, model) or interpretForm(form.arg2, model)
    if form.isa(Implies): return not interpretForm(form.arg1, model) or interpretForm(form.arg2, model)
    raise Exception("Unhandled: %s" % form)

# Conjunction
def interpretForms(forms, model):
    return all(interpretForm(form, model) for form in forms)

############################################################

# A Derivation is a tree where each node corresponds to the application of a rule.
# For any Formula, we can extract a set of categories.
# Rule arguments are labeled with category.
class Derivation:
    def __init__(self, form, children, cost, derived):
        self.form = form
        self.children = children
        self.cost = cost
        self.permanent = False  # Marker for being able to extract.
        self.derived = derived  # Whether this was derived (as opposed to added by the user).
    def __repr__(self): return 'Derivation(%s, cost=%s, permanent=%s, derived=%s)' % (self.form, self.cost, self.permanent, self.derived)

# Possible responses to queries to the knowledge base
ENTAILMENT = "ENTAILMENT"
CONTINGENT = "CONTINGENT"
CONTRADICTION = "CONTRADICTION"

# A response to a KB query
class KBResponse:
    # query: what the query is (just a string description for printing)
    # modify: whether we modified the knowledge base
    # status: one of the ENTAILMENT, CONTINGENT, CONTRADICTION
    # trueModel: if available, a model consistent with the KB for which the the query is true
    # falseModel: if available, a model consistent with the KB for which the the query is false
    def __init__(self, query, modify, status, trueModel, falseModel):
        self.query = query
        self.modify = modify
        self.status = status
        self.trueModel = trueModel
        self.falseModel = falseModel

    def show(self, verbose=1):
        padding = '>>>>>'
        print(padding + ' ' + self.responseStr())
        if verbose >= 1:
            print(('Query: %s[%s]' % ('TELL' if self.modify else 'ASK', self.query)))
            if self.trueModel:
                print('An example of a model where query is TRUE:')
                printModel(self.trueModel)
            if self.falseModel:
                print('An example of a model where query is FALSE:')
                printModel(self.falseModel)

    def responseStr(self):
        if self.status == ENTAILMENT:
            if self.modify: return 'I already knew that.'
            else: return 'Yes.'
        elif self.status == CONTINGENT:
            if self.modify: return 'I learned something.'
            else: return 'I don\'t know.'
        elif self.status == CONTRADICTION:
            if self.modify: return 'I don\'t buy that.'
            else: return 'No.'
        else:
            raise Exception("Invalid status: %s" % self.status)

    def __repr__(self): 
        return self.responseStr()

def showKBResponse(response, verbose=1):
    if isinstance(response, KBResponse):
        response.show(verbose)
    else:
        items = [(obj, r.status) for ((var, obj), r) in list(response.items())]
        print(('Yes: %s' % rstr([obj for obj, status in items if status == ENTAILMENT])))
        print(('Maybe: %s' % rstr([obj for obj, status in items if status == CONTINGENT])))
        print(('No: %s' % rstr([obj for obj, status in items if status == CONTRADICTION])))

# A KnowledgeBase is a set collection of Formulas.
# Interact with it using
# - addRule: add inference rules
# - tell: modify the KB with a new formula.
# - ask: query the KB about 
class KnowledgeBase:
    def __init__(self, standardizationRule, rules, modelChecking, verbose=0):
        # Rule to apply to each formula that's added to the KB (None is possible).
        self.standardizationRule = standardizationRule

        # Set of inference rules
        self.rules = rules

        # Use model checking as opposed to applying rules.
        self.modelChecking = modelChecking

        # For debugging
        self.verbose = verbose 

        # Formulas that we believe are true (used when not doing model checking).
        self.derivations = {}  # Map from Derivation key (logical form) to Derivation

    # Add a formula |form| to the KB if it doesn't contradict.  Returns a KBResponse.
    def tell(self, form):
        return self.query(form, modify=True)

    # Ask whether the logical formula |form| is True, False, or unknown based
    # on the KB.  Returns a KBResponse.
    def ask(self, form):
        return self.query(form, modify=False)

    def dump(self):
        print(('==== Knowledge base [%d derivations] ===' % len(self.derivations)))
        for deriv in list(self.derivations.values()):
            print((('-' if deriv.derived else '*'), deriv if self.verbose >= 2 else deriv.form))

    ####### Internal functions

    # Returns a KBResponse or if there are free variables, a mapping from (var, obj) => query without that variable.
    def query(self, form, modify):
        #print 'QUERY', form
        # Handle wh-queries: try all possible values of the free variable, and recurse on query().
        freeVars = allFreeVars(form)
        if len(freeVars) > 0:
            if modify:
                raise Exception("Can't modify database with a query with free variables: %s" % form)
            var = freeVars[0]
            allForms = AndList([deriv.form for deriv in list(self.derivations.values())])
            if allForms == AtomTrue: return {}  # Weird corner case
            objects = allConstants(allForms) 
            # Try binding |var| to |obj|
            response = {}
            for obj in objects:
                response[(var, obj)] = self.query(substituteFreeVars(form, var, obj), modify)
            return response

        # Assume no free variables from here on...
        formStr = '%s, standardized: %s' % (form, rstr(self.standardize(form)))

        # Models to serve as supporting evidence
        falseModel = None  # Makes the query false
        trueModel = None  # Makes the query true

        # Add Not(form)
        if not self.addAxiom(Not(form)):
            self.removeTemporary()
            status = ENTAILMENT
        else:
            # Inconclusive...
            falseModel = self.consistentModel
            self.removeTemporary()

            # Add form
            if self.addAxiom(form):
                if modify:
                    self.makeTemporaryPermanent()
                else:
                    self.removeTemporary()
                trueModel = self.consistentModel
                status = CONTINGENT
            else:
                self.removeTemporary()
                status = CONTRADICTION

        return KBResponse(query = formStr, modify = modify, status = status, trueModel = trueModel, falseModel = falseModel)

    # Apply the standardization rule to |form|.
    def standardize(self, form):
        if self.standardizationRule:
            return self.standardizationRule.applyRule(form)
        return [form]

    # Return whether adding |form| is consistent with the current knowledge base.
    # Add |form| to the knowledge base if we can.  Note: this is done temporarily!
    # Just calls addDerivation.
    def addAxiom(self, form):
        self.consistentModel = None
        for f in self.standardize(form):
            if f == AtomFalse: return False
            if f == AtomTrue: continue
            deriv = Derivation(f, children = [], cost = 0, derived = False)
            if not self.addDerivation(deriv): return False
        return True

    # Return whether the Derivation is consistent with the KB.
    def addDerivation(self, deriv):
        # Derived a contradiction
        if deriv.form == AtomFalse: return False

        key = deriv.form
        oldDeriv = self.derivations.get(key)
        maxCost = 100
        if oldDeriv == None and deriv.cost <= maxCost:
        #if oldDeriv == None or (deriv.cost < oldDeriv.cost and (deriv.permanent >= oldDeriv.permanent)):
            #print 'UPDATE %s %s' % (deriv, oldDeriv)
            #self.dump()
            # Something worth updating
            self.derivations[key] = deriv
            if self.verbose >= 3: print(('add %s [%s derivations]' % (deriv, len(self.derivations))))

            if self.modelChecking:
                allForms = [deriv.form for deriv in list(self.derivations.values())]
                models = performModelChecking(allForms, findAll=False, verbose=self.verbose)
                if len(models) == 0: return False
                else: self.consistentModel = models[0]

            # Apply rules forward
            if not self.applyUnaryRules(deriv): return False
            for key2, deriv2 in list(self.derivations.items()):
                if not self.applyBinaryRules(deriv, deriv2): return False
                if not self.applyBinaryRules(deriv2, deriv): return False

        return True

    # Raise an exception if |formulas| is not a list of Formulas.
    def ensureFormulas(self, rule, formulas):
        if isinstance(formulas, list) and all(formula == False or isinstance(formula, Formula) for formula in formulas):
            return formulas
        raise Exception('Expected list of Formulas, but %s returned %s' % (rule, formulas))

    # Return whether everything is okay (no contradiction).
    def applyUnaryRules(self, deriv):
        for rule in self.rules:
            if not isinstance(rule, UnaryRule): continue
            for newForm in self.ensureFormulas(rule, rule.applyRule(deriv.form)):
                if not self.addDerivation(Derivation(newForm, children = [deriv], cost = deriv.cost + 1, derived = True)):
                    return False
        return True

    # Return whether everything is okay (no contradiction).
    def applyBinaryRules(self, deriv1, deriv2):
        for rule in self.rules:
            if not isinstance(rule, BinaryRule): continue
            if rule.symmetric() and str(deriv1.form) >= str(deriv2.form): continue  # Optimization
            for newForm in self.ensureFormulas(rule, rule.applyRule(deriv1.form, deriv2.form)):
                if not self.addDerivation(Derivation(newForm, children = [deriv1, deriv2], cost = deriv1.cost + deriv2.cost + 1, derived = True)):
                    return False
        return True

    # Remove all the temporary derivations from the KB.
    def removeTemporary(self):
        for key, value in list(self.derivations.items()):
            if not value.permanent:
                del self.derivations[key]

    # Mark all the derivations marked temporary to permanent.
    def makeTemporaryPermanent(self):
        for deriv in list(self.derivations.values()):
            deriv.permanent = True

# Create an empty knowledge base equipped with the usual inference rules.
def createResolutionKB():
    return KnowledgeBase(standardizationRule = ToCNFRule(), rules = [ResolutionRule()], modelChecking = False)

def createModelCheckingKB():
    return KnowledgeBase(standardizationRule = None, rules = [], modelChecking = True)
