from sat_problem_solver import LSAT_Z3_Program

if __name__=="__main__":
    completion = '''# Declarations
people = EnumSort([Vladimir, Wendy])
meals = EnumSort([breakfast, lunch, dinner, snack])
foods = EnumSort([fish, hot_cakes, macaroni, omelet, poached_eggs])
eats = Function([people, meals] -> [foods])

# Constraints
ForAll([m:meals], eats(Vladimir, m) != eats(Wendy, m)) ::: At no meal does Vladimir eat the same kind of food as Wendy
ForAll([p:people, f:foods], Count([m:meals], eats(p, m) == f) <= 1) ::: Neither of them eats the same kind of food more than once during the day
ForAll([p:people], Or(eats(p, breakfast) == hot_cakes, eats(p, breakfast) == poached_eggs, eats(p, breakfast) == omelet)) ::: For breakfast, each eats exactly one of the following: hot cakes, poached eggs, or omelet
ForAll([p:people], Or(eats(p, lunch) == fish, eats(p, lunch) == hot_cakes, eats(p, lunch) == macaroni, eats(p, lunch) == omelet)) ::: For lunch, each eats exactly one of the following: fish, hot cakes, macaroni, or omelet
ForAll([p:people], Or(eats(p, dinner) == fish, eats(p, dinner) == hot_cakes, eats(p, dinner) == macaroni, eats(p, dinner) == omelet)) ::: For dinner, each eats exactly one of the following: fish, hot cakes, macaroni, or omelet
ForAll([p:people], Or(eats(p, snack) == fish, eats(p, snack) == omelet)) ::: For a snack, each eats exactly one of the following: fish or omelet
eats(Wendy, lunch) == omelet ::: Wendy eats an omelet for lunch

# Options
Question ::: Vladimir must eat which one of the following foods?
is_valid(Exists([m:meals], eats(Vladimir, m) == fish)) ::: (A)
is_valid(Exists([m:meals], eats(Vladimir, m) == hot_cakes)) ::: (B)
is_valid(Exists([m:meals], eats(Vladimir, m) == macaroni)) ::: (C)
is_valid(Exists([m:meals], eats(Vladimir, m) == omelet)) ::: (D)
is_valid(Exists([m:meals], eats(Vladimir, m) == poached_eggs)) ::: (E)'''

    completion_2 = '''# declare variables
technicians = EnumSort([Stacy, Urma, Wim, Xena, Yolanda, Zane])
machines = EnumSort([radios, televisions, VCRs])
repairs = Function([technicians, machines] -> [bool])

# constraints
# each technician repairs machines of at least one of the following three types
ForAll([t:technicians], Count([m:machines], repairs(t, m)) >= 1)

# Xena and exactly three other technicians repair radios
And(repairs(Xena, radios), Count([t:technicians], And(t != Xena, repairs(t, radios))) == 3)

# Yolanda repairs both televisions and VCRs
And(repairs(Yolanda, televisions), repairs(Yolanda, VCRs))

# Stacy does not repair any type of machine that Yolanda repairs
ForAll([m:machines], Implies(repairs(Yolanda, m), Not(repairs(Stacy, m))))

# Zane repairs more types of machines than Yolanda repairs
Count([m:machines], repairs(Zane, m)) > Count([m:machines], repairs(Yolanda, m))

# Wim does not repair any type of machine that Stacy repairs
ForAll([m:machines], Implies(repairs(Stacy, m), Not(repairs(Wim, m))))

# Urma repairs exactly two types of machines
Count([m:machines], repairs(Urma, m)) == 2

# Which one of the following pairs of technicians could repair all and only the same types of machines as each other?
# we check whether the options can possibly be true
# (A)
is_sat(ForAll([m:machines], repairs(Stacy, m) == repairs(Urma, m)))
# (B)
is_sat(ForAll([m:machines], repairs(Urma, m) == repairs(Yolanda, m)))
# (C)
is_sat(ForAll([m:machines], repairs(Urma, m) == repairs(Xena, m)))
# (D)
is_sat(ForAll([m:machines], repairs(Wim, m) == repairs(Xena, m)))
# (E)
is_sat(ForAll([m:machines], repairs(Xena, m) == repairs(Yolanda, m)))'''

    completion_3 = '''# declare variables
CD_types = EnumSort([jazz, opera, pop, rap, soul])
new_used = EnumSort([new, used])
on_sale = Function([CD_types, new_used] -> [bool])

# constraints
# Used pop is on sale; new opera is not
And(on_sale(pop, used), Not(on_sale(opera, new)))

# If both types of pop are on sale, then all soul is
Implies(And(on_sale(pop, new), on_sale(pop, used)), And(on_sale(soul, new), on_sale(soul, used)))

# If both types of jazz are on sale, then no rap is
Implies(And(on_sale(jazz, new), on_sale(jazz, used)), ForAll([nu:new_used], Not(on_sale(rap, nu))))

# If neither type of jazz is on sale, then new pop is
Implies(And(Not(on_sale(jazz, new)), Not(on_sale(jazz, used))), on_sale(pop, new))

# If either type of rap is on sale, then no soul is
Implies(Or(on_sale(rap, new), on_sale(rap, used)), ForAll([nu:new_used], Not(on_sale(soul, nu))))

# If exactly four of the five types of used CDs are the only CDs on sale
And(Count([t:CD_types], on_sale(t, used)) == 4, Count([t:CD_types], on_sale(t, new)) == 0)

# we check which one of the following could be true?
# (A) Used jazz is not on sale
is_sat(Not(on_sale(jazz, used)))

# (B) Used opera is not on sale
is_sat(Not(on_sale(opera, used)))

# (C) Used rap is not on sale
is_sat(Not(on_sale(rap, used)))

# (D) Neither type of jazz is on sale
is_sat(And(Not(on_sale(jazz, new)), Not(on_sale(jazz, used))))

# (E) Neither type of rap and neither type of soul is on sale
is_sat(And(Not(on_sale(rap, new)), Not(on_sale(rap, used)), Not(on_sale(soul, new)), Not(on_sale(soul, used))))'''

    completion_4 = '''# Declarations
children = EnumSort([Fred, Juan, Marc, Paul, Nita, Rachel, Trisha])
lockers = EnumSort([1, 2, 3, 4, 5])
genders = EnumSort([boy, girl])
assigned = Function([children] -> [lockers])
gender = Function([children] -> [genders])

# Constraints
ForAll([l:lockers], Or(Exists([c:children], assigned(c) == l), Exists([c1:children, c2:children], And(c1 != c2, And(assigned(c1) == l, assigned(c2) == l, gender(c1) != gender(c2)))))) ::: Each locker must be assigned to either one or two children, and each child must be assigned to exactly one locker
Exists([c:children, l:lockers], And(assigned(Juan) == l, assigned(c) == l, c != Juan)) ::: Juan must share a locker
ForAll([l:lockers], Or(assigned(Rachel) != l, Not(Exists([c:children], And(c != Rachel, assigned(c) == l))))) ::: Rachel cannot share a locker
ForAll([l:lockers], Implies(assigned(Nita) == l, And(assigned(Trisha) != l - 1, assigned(Trisha) != l + 1))) ::: Nita's locker cannot be adjacent to Trisha's locker
assigned(Fred) == 3 ::: Fred must be assigned to locker 3
And(assigned(Trisha) == 3, assigned(Marc) == 1, ForAll([c:children], Implies(c != Marc, assigned(c) != 1))) ::: if Trisha is assigned to locker 3 and Marc alone is assigned to locker 1

# Options
Question ::: If Trisha is assigned to locker 3 and Marc alone is assigned to locker 1, then which one of the following must be true?
is_valid(Exists([c:children], And(assigned(Juan) == 4, assigned(c) == 4, c != Juan))) ::: (A)
is_valid(Exists([c:children], And(assigned(Juan) == 5, assigned(c) == 5, c != Juan))) ::: (B)
is_valid(assigned(Paul) == 2) ::: (C)
is_valid(assigned(Rachel) == 2) ::: (D)
is_valid(assigned(Rachel) == 5) ::: (E)'''

    completion_5 = '''# Declarations
vehicles = EnumSort([hatchback, limousine, pickup, roadster, sedan, van])
# 1 for Monday, 6 for Saturday
days = EnumSort([1, 2, 3, 4, 5, 6])
serviced = Function([vehicles] -> [days])
ForAll([v:vehicles], And(1 <= serviced(v), serviced(v) <= 6))

# Constraints
Exists([v:vehicles], serviced(v) > serviced(hatchback)) ::: At least one of the vehicles is serviced later in the week than the hatchback
And(serviced(roadster) > serviced(van), serviced(roadster) < serviced(hatchback)) ::: The roadster is serviced later in the week than the van and earlier in the week than the hatchback
Xor(Abs(serviced(pickup) - serviced(van)) == 1, Abs(serviced(pickup) - serviced(sedan)) == 1) ::: Either the pickup and the van are serviced on consecutive days, or the pickup and the sedan are serviced on consecutive days, but not both
Xor(serviced(sedan) < serviced(pickup), serviced(sedan) < serviced(limousine)) ::: The sedan is serviced earlier in the week than the pickup or earlier in the week than the limousine, but not both

# Options
Question ::: Which one of the following could be the list of the vehicles serviced on Tuesday, Wednesday, and Friday, listed in that order?
is_sat(And(serviced(pickup) == 2, serviced(hatchback) == 3, serviced(limousine) == 5)) ::: (A)
is_sat(And(serviced(pickup) == 2, serviced(roadster) == 3, serviced(hatchback) == 5)) ::: (B)
is_sat(And(serviced(sedan) == 2, serviced(limousine) == 3, serviced(hatchback) == 5)) ::: (C)
is_sat(And(serviced(van) == 2, serviced(limousine) == 3, serviced(hatchback) == 5)) ::: (D)
is_sat(And(serviced(van) == 2, serviced(roadster) == 3, serviced(limousine) == 5)) ::: (E)'''

    completion_6 = '''# Declarations
vehicles = EnumSort([hatchback, limousine, pickup, roadster, sedan, van])
days = IntSort([Monday, Tuesday, Wednesday, Thursday, Friday, Saturday])
serviced = Function([vehicles] -> [days])
ForAll([v:vehicles], And(1 <= serviced(v), serviced(v) <= 6))

# Constraints
And(Monday == 1, Tuesday == 2, Wednesday == 3, Thursday == 4, Friday == 5)
Exists([v:vehicles], serviced(v) > serviced(hatchback)) ::: At least one of the vehicles is serviced later in the week than the hatchback
And(serviced(roadster) > serviced(van), serviced(roadster) < serviced(hatchback)) ::: The roadster is serviced later in the week than the van and earlier in the week than the hatchback
Xor(Abs(serviced(pickup) - serviced(van)) == 1, Abs(serviced(pickup) - serviced(sedan)) == 1) ::: Either the pickup and the van are serviced on consecutive days, or the pickup and the sedan are serviced on consecutive days, but not both
Xor(serviced(sedan) < serviced(pickup), serviced(sedan) < serviced(limousine)) ::: The sedan is serviced earlier in the week than the pickup or earlier in the week than the limousine, but not both

# Options
Question ::: Which one of the following could be the list of the vehicles serviced on Tuesday, Wednesday, and Friday, listed in that order?
is_sat(And(serviced(pickup) == Tuesday, serviced(hatchback) == Wednesday, serviced(limousine) == Friday)) ::: (A)
is_sat(And(serviced(pickup) == Tuesday, serviced(roadster) == Wednesday, serviced(hatchback) == Friday)) ::: (B)
is_sat(And(serviced(sedan) == Tuesday, serviced(limousine) == Wednesday, serviced(hatchback) == Friday)) ::: (C)
is_sat(And(serviced(van) == Tuesday, serviced(limousine) == Wednesday, serviced(hatchback) == Friday)) ::: (D)
is_sat(And(serviced(van) == Tuesday, serviced(roadster) == Wednesday, serviced(limousine) == Friday)) ::: (E)'''

    completion_7 = '''# Declarations
days = IntSort([Monday, Tuesday, Wednesday, Thursday, Friday])
divisions = EnumSort([Operations, Production, Sales])
toured = Function([days] -> [divisions])

# Constraints
And(Monday == 1, Tuesday == 2, Wednesday == 3, Thursday == 4, Friday == 5)
Count([d:days], toured(d) == Operations) >= 1 ::: Each division is toured at least once
Count([d:days], toured(d) == Production) >= 1 ::: Each division is toured at least once
Count([d:days], toured(d) == Sales) >= 1 ::: Each division is toured at least once
toured(Monday) != Operations ::: The Operations division is not toured on Monday
toured(Wednesday) != Production ::: The Production division is not toured on Wednesday
And(Or(toured(Monday) == Sales, toured(Tuesday) == Sales), Or(toured(Tuesday) == Sales, toured(Wednesday) == Sales), Or(toured(Wednesday) == Sales, toured(Thursday) == Sales), Or(toured(Thursday) == Sales, toured(Friday) == Sales)) ::: The Sales division is toured on two consecutive days
Count([d:days], toured(d) == Sales) == 2 ::: The Sales division is toured on two consecutive days, and on no other days
Implies(toured(Thursday) == Operations, toured(Friday) == Production) ::: If the Operations division is toured on Thursday, then the Production division is toured on Friday
toured(Monday) != toured(Tuesday) ::: if in the week's tour schedule the division that is toured on Monday is not the division that is toured on Tuesday

# Options
Question ::: Which one of the following could be true of the week's schedule?
is_sat(Exists([d:days], And(toured(d) == Sales, ForAll([d2:days], Implies(d2 < d, toured(d2) != Production))))) ::: (A)
is_sat(Exists([d:days], And(toured(d) == Operations, ForAll([d2:days], Implies(d2 < d, toured(d2) != Production))))) ::: (B)
is_sat(toured(Monday) == Sales) ::: (C)
is_sat(toured(Tuesday) == Production) ::: (D)
is_sat(toured(Wednesday) == Operations) ::: (E)'''

    z3_program = LSAT_Z3_Program(completion_6, 'AR-LSAT')
    ans, error_message = z3_program.execute_program()
    #print(ans)
    print(error_message)
    print(z3_program.standard_code)