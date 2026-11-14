"""Random input sessions for the fuzz test.

Lines are built from the call names in sd_calls.txt, the concept names in
sdctable.cpp, and the selector names in sdtables.cpp, then mutated.  At a
few random points in each line, the session presses Tab, space, hyphen, or
'?', and then Enter on the whole line.  A session is reproducible from its
seed, as long as those three source files don't change.
"""

import os
import random
import re

DEL = '\x7f' * 460   # more than the 400-character input limit

SETUPS = ['heads start\n', 'heads 1p2p\n', 'heads 1p2p\ntouch 1/4\n',
          'heads 1p2p\nswing thru\n', 'heads 1p2p\ntouch 1/4\nscoot back\n',
          'sides start\n', 'just as they are\n']
WEIRD = ['<anything>', '<anyone>', '<atc>', '<n>', '<n/4>', '<direction>', "er's", 'ers',
         " er'", '@0', '@6', '@', '#', '[', ']', '[[', ']]', '[]', '[ ]', ',', "'", '-', '--',
         '  ', '"', '\\', '%s', '%n', '\x80', '\xe9', '\xff', '\xff\xff', '<', '>', '<<', '>>']
DIRECTIONS = ['left', 'right', 'in', 'out', 'back', 'zig-zag', 'zag-zig', 'the music']
NUMBERS = ['1', '2', '3', '4', '5', '1/4', '1/2', '3/4', '1-1/2', 'half', 'quarter', 'full',
           'three quarter', '7/8']
TAGGERS = ['bits and pieces', 'checkmate', 'motivate', 'vertical', 'tag', '<atc>']
TAILS = [' and roll', ' and spread', '', " er's motivate", ' chain thru', ' like a recoil']
KEYS = ['\t', '\t', '\t', ' ', ' ', '-', '?']


class Generator:
    def __init__(self, src_dir):
        def read(name):
            with open(os.path.join(src_dir, name), encoding='latin-1') as f:
                return f.read()

        self.calls = re.findall(r'^call "([^"]*)"', read('sd_calls.txt'), re.M)
        self.concepts = [c.lower() for c in
                         re.findall(r'^\s*\{"([^"]+)",\s*concept_', read('sdctable.cpp'), re.M)]
        sel = re.findall(r'\{"([^"]*)",\s*"[^"]*",\s*"[^"]*",\s*"[^"]*",\s*selector_',
                         read('sdtables.cpp'))
        self.selectors = [s for s in sel if '@' not in s and '?' not in s]
        if not (self.calls and self.concepts and self.selectors):
            raise RuntimeError(f'could not read the call, concept, or selector names in {src_dir}')

    def expand(self, name, rng, depth=0):
        """Fill in the @ escapes of a call name with random text."""
        out, i = [], 0
        while i < len(name):
            c = name[i]
            if c == '@' and i + 1 < len(name):
                k = name[i + 1]
                i += 2
                if k in '0mT':
                    if depth < 3 and rng.random() < .8:
                        sub = self.expand(rng.choice(self.calls), rng, depth + 1)
                    else:
                        sub = rng.choice(WEIRD)
                    out.append('[' + sub + ']')
                    if k == 'T' and rng.random() < .5:
                        out.append(rng.choice([" er's", ' ers', " er'", "er's", ' e', '']))
                elif k in '6kKV':
                    out.append(rng.choice(self.selectors))
                elif k in '9uabBD':
                    out.append(rng.choice(NUMBERS))
                elif k == 'h':
                    out.append(rng.choice(DIRECTIONS))
                elif k in 'vwxy':
                    out.append(rng.choice(TAGGERS))
                elif k in 'On':
                    j = name.find('@', i)
                    i = len(name) if j < 0 else j + 2
            else:
                out.append(c)
                i += 1
        return ''.join(out)

    def mutate(self, s, rng):
        s = list(s)
        for _ in range(rng.choice([0, 0, 1, 1, 2, 3, 6])):
            if not s:
                s = list(rng.choice(WEIRD))
            op = rng.random()
            pos = rng.randrange(len(s) + 1)
            if op < .25:
                s[pos:pos] = list(rng.choice(WEIRD))
            elif op < .45 and s:
                del s[pos:pos + rng.randint(1, 6)]
            elif op < .6:
                word = rng.choice(self.calls + self.concepts + self.selectors).replace('@', '')
                s[pos:pos] = list(' ' + word + ' ')
            elif op < .7 and s:
                j = min(len(s) - 1, pos)
                s[j] = s[j].upper() if s[j].isascii() else s[j]
            elif op < .8:
                s[pos:pos] = rng.choice(['[', ']'])
            elif op < .9 and s:
                k = rng.randrange(len(s))
                s[k] = chr(rng.randrange(32, 256))
            else:
                s = s[::-1] if rng.random() < .1 else s + list(' ' + rng.choice(self.calls).split('@')[0])
        return ''.join(s)

    def line(self, rng):
        kind = rng.random()
        if kind < .35:
            text = self.expand(rng.choice(self.calls), rng)
        elif kind < .55:
            text = rng.choice(self.concepts) + ' ' + self.expand(rng.choice(self.calls), rng)
        elif kind < .7:
            n = rng.randint(1, 4)
            text = '[' * n + self.expand(rng.choice(self.calls), rng) + \
                ''.join(']' + rng.choice(TAILS) for _ in range(rng.randint(0, n + 2)))
        elif kind < .8:
            words = self.calls + self.concepts + self.selectors + WEIRD
            text = ' '.join(rng.choice(words).replace('@', rng.choice(['', '@']))
                            for _ in range(rng.randint(1, 8)))
        elif kind < .9:
            text = ''.join(rng.choice('[] abcdeghilmnorstuwy\'",-<>@#0123456789/')
                           for _ in range(rng.randint(1, 60)))
        else:
            # Long lines, past the input limit.
            text = self.expand(rng.choice(self.calls), rng) * rng.randint(1, 40)
        text = self.mutate(text, rng)
        for c in '\n\r?!\x1b':
            text = text.replace(c, ' ' if c in '\n\r' else '')
        return text

    def session(self, seed, nlines=5):
        """The whole input for one session, ending with "exit the program"."""
        rng = random.Random(seed)
        parts = [rng.choice(SETUPS)]
        for _ in range(nlines):
            ln = self.line(rng)
            cuts = sorted(set(rng.sample(range(1, len(ln) + 1), min(len(ln), 6)))) if ln else []
            for k in cuts:
                parts.append(ln[:k] + rng.choice(KEYS) + DEL)
            parts.append(ln + '\n' + DEL)
        parts.append('exit the program\ny\n')
        return ''.join(parts)
