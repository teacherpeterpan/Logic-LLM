import nltk
import re

class FOL_Parser:
    def __init__(self) -> None:
        self.op_ls = ['⊕', '∨', '∧', '→', '↔', '∀', '∃', '¬', '(', ')', ',']

        self.sym_reg = re.compile(r'[^⊕∨∧→↔∀∃¬(),]+')

        # modified a bit. 
        self.cfg_template = """
        S -> F | Q F | '¬' S | '(' S ')'
        Q -> QUANT VAR | QUANT VAR Q
        F -> '¬' '(' F ')' | '(' F ')' | F OP F | L
        OP -> '⊕' | '∨' | '∧' | '→' | '↔'
        L -> '¬' PRED '(' TERMS ')' | PRED '(' TERMS ')'
        TERMS -> TERM | TERM ',' TERMS
        TERM -> CONST | VAR
        QUANT -> '∀' | '∃'
        """

    def parse_text_FOL_to_tree(self, rule_str):
        """
            Parse a text FOL rule into nltk.tree
            
            Returns: nltk.tree, or None if the parse fails
        """
        ## NOTE: currenly we don't support FOL string that does not follow the grammar defined above. 
        # rule_str = self.reorder_quantifiers(rule_str)
    
        r, parsed_fol_str = self.msplit(rule_str)
        cfg_str = self.make_cfg_str(r)

        grammar = nltk.CFG.fromstring(cfg_str)
        parser = nltk.ChartParser(grammar)
        # note this function might run forever if the string is not parsable
        tree = parser.parse_one(r) 
        
        return tree
    
    def reorder_quantifiers(self, rule_str):
        matches = re.findall(r'[∃∀]\w', rule_str)
        for match in matches[::-1]:
            rule_str = '%s ' % match + rule_str.replace(match, '', 1)
        return rule_str

    def msplit(self, s):
        for op in self.op_ls:
            s = s.replace(op, ' %s ' % op)
        r = [e.strip() for e in s.split()]
        #remove ' from the string if it contains any: this causes error in nltk cfg parsing
        r = [e.replace('\'', '') for e in r]
        r = [e for e in r if e != '']
        
        # deal with symbols with spaces like "dc universe" and turn it to "DcUniverse"
        res = []
        cur_str_ls = []
        for e in r:
            if (len(e) > 1) and self.sym_reg.match(e):            
                cur_str_ls.append(e[0].upper() + e[1:])            
            else:
                if len(cur_str_ls) > 0:
                    res.extend([''.join(cur_str_ls), e])
                else:
                    res.extend([e])
                cur_str_ls = []
        if len(cur_str_ls) > 0:
            res.append(''.join(cur_str_ls))
        
        # re-generate the FOL string
        make_str_ls = []
        for ind, e in enumerate(r):
            if re.match(r'[⊕∨∧→↔]', e):
                make_str_ls.append(' %s ' % e)
            elif re.match(r',', e):
                make_str_ls.append('%s ' % e)
            # a logical variable
            elif (len(e) == 1) and re.match(r'\w', e):
                if ((ind - 1) >= 0) and ((r[ind-1] == '∃') or (r[ind-1] == '∀')):
                    make_str_ls.append('%s ' % e)
                else:
                    make_str_ls.append(e)
            else:
                make_str_ls.append(e)
        
        return res, ''.join(make_str_ls)


    def make_cfg_str(self, token_ls):
        """
        NOTE: since nltk does not support reg strs like \w+, we cannot separately recognize VAR, PRED, and CONST.
        Instead, we first allow VAR, PRED, and CONST to be matched with all symbols found in the FOL; once the tree is
        parsered, we then go back and figure out the exact type of each symbols
        """
        sym_ls = list(set([e for e in token_ls if self.sym_reg.match(e)]))
        sym_str = ' | '.join(["'%s'" % s for s in sym_ls])
        cfg_str = self.cfg_template + 'VAR -> %s\nPRED -> %s\nCONST -> %s' % (sym_str,sym_str,sym_str)
        return cfg_str

    def find_variables(self, lvars, tree):
        if isinstance(tree, str):
            return
        
        if tree.label() == 'VAR':
            lvars.add(tree[0])
            return
        
        for child in tree:
            self.find_variables(lvars, child)

    def symbol_resolution(self, tree):
        lvars, consts, preds = set(), set(), set()
        self.find_variables(lvars, tree)
        self.preorder_resolution(tree, lvars, consts, preds)
        return lvars, consts, preds


    def preorder_resolution(self, tree, lvars, consts, preds):
        # reached terminal nodes
        if isinstance(tree, str):
            return
        
        if tree.label() == 'PRED':
            preds.add(tree[0])
            return
        
        if tree.label() == 'TERM':
            sym = tree[0][0]
            if sym in lvars:
                tree[0].set_label('VAR')
            else:
                tree[0].set_label('CONST')
                consts.add(sym)
            return
        
        for child in tree:
            self.preorder_resolution(child, lvars, consts, preds)

if __name__ == '__main__':
    # str_fol = '\u2200x (Dog(x) \u2227 WellTrained(x) \u2227 Gentle(x) \u2192 TherapyAnimal(x))'
    str_fol = '\u2200x (Athlete(x) \u2227 WinsGold(x, olympics) \u2192 OlympicChampion(x))'
    # str_fol = '¬∀x ∃x(Movie(x) → HappyEnding(x))'
    
    parser = FOL_Parser()

    tree = parser.parse_text_FOL_to_tree(str_fol)
    print(tree)
    tree.pretty_print()
    
    lvars, consts, preds = parser.symbol_resolution(tree)
    print('lvars: ', lvars)
    print('consts: ', consts)
    print('preds: ', preds)