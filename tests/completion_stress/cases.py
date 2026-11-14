"""Inputs for the completion stress tests.

Everything here was written for these tests.  Nothing is taken from the
regression suite, which is read at run time from the sd-regression checkout.
"""

import fnmatch

# Starting formations, as the lines typed to reach them.
SETUPS = {
    'lines': 'heads 1p2p\n',
    'waves': 'heads 1p2p\ntouch 1/4\n',
    'qtag': 'heads 1p2p\ntouch 1/4\nscoot back\n',
    'tidal': 'heads 1p2p\nswing thru\n',
    'squared': 'heads start\n',
}

# Lines with nested brackets.  The last three of the first block are the
# examples that made the parser slow before the parser optimizations.
LINES = [
    '[[swing thru] and roll] and roll',
    '[[[swing thru] and roll] and roll] and spread',
    "[[step and fold] er's motivate]",
    '[[step and fold] ers motivate]',
    '[[ah so] em up] and roll',
    "[[ah so] 'em up] and roll",
    '[<anything> and roll] and spread',
    '[[swing thru] o late] and roll',
    '[[swing thru] and the boys roll] and roll',
    '[[swing thru] and rally but [hinge]] and roll',
    'fascinating [[ah so] and cross] and roll',
    'initially [[step and fold] and roll] and plenty, ends face in, '
    'replace the star turns with [trade circulate]',
    "initially rotary [[step and fold] er's motivate]",
    "rotary [[step and fold] er's motivate]",
    'cross [[swing thru] and roll] and spread',
    'left [[swing thru] and roll] and spread',
    '[swing thru] and roll',
    '[[swing thru] chain thru] and roll',
    '[[hinge] like a recoil] and roll',
    'dodge [[counter rotate] and roll]',
    'busy [[turn and deal] by golly]',
    'make a pass but [[chain down the line] and roll]',
    '[[[[swing thru] and roll] and roll] and roll] and roll',
    'boys [[swing thru] and roll] and roll',
    '[[boys run] and roll] and roll',
    '[[the boys run] and roll] cover up',
    '[[mark time] and roll] cover up',
    "[[swing thru] and roll] er's cover up",
]
LINE_SETUPS = ['lines', 'waves', 'qtag', 'tidal']

# Example calls for the speed check, typed in waves.
SPEED_SETUP = 'waves'
SPEED_LINES = [
    "initially rotary [[step and fold] er's motivate]",
    'fascinating [[[ah so] and cross] and roll] and spread',
    'initially [[step and fold] and roll] and plenty, ends face in, '
    'replace the star turns with [trade circulate]',
]

# The yield test: an inner call, some of them marked yield_if_ambiguous
# in the database, in "[[<inner>] and circle <n>", after an outer call or
# concept.
YIELD_OUTER = ['', 'busy ', 'dodge ', 'tunnel ', 'catch ', 'fascinating ',
               'make a pass but ', 'rotary ', 'initially ', 'finally ', 'twice ',
               'split catch ', 'replace the column but ', 'spin the pulley but ',
               'tally ho but ', 'motivate but ']
YIELD_INNER = ['boys run', 'rotary circulate', 'girls run', 'centers run', 'tag',
               '1/2 tag', 'heads run']
YIELD_TAILS = [' and circle 1/4', ' and circle 1/2', ' and circle 3/4']
YIELD_SETUPS = ['lines', 'waves', 'tidal', 'qtag', 'squared']


def yield_lines():
    return [o + '[[' + i + ']' + t
            for o in YIELD_OUTER for i in YIELD_INNER for t in YIELD_TAILS]


# The level and flags that sd-regression's doit.sh uses for each group of
# tests.  The xr*t tests run under a different script and are left out.
REGRESSION_RULES = [
    ('t*t', 'c4x', []), ('o*t', 'c4x', ['-warn_on_overflow']), ('w*t', 'c4x', []),
    ('c*t', 'c4', ['-warn_on_overflow']), ('k*t', 'c4x', ['-warn_on_overflow']),
    ('n*t', 'c4x', []), ('r*t', 'c4', ['-warn_on_overflow']),
    ('l*t', 'c4x', ['-warn_on_overflow']), ('g*t', 'c4x', ['-warn_on_overflow']),
    ('z*t', 'c4', ['-warn_on_overflow']), ('u*t', 'c4', ['-warn_on_overflow']),
    ('v*t', 'c4x', ['-warn_on_overflow']), ('p*t', 'c4x', []),
    ('b*t', 'c4', ['-warn_on_overflow']), ('y*t', 'c4', ['-warn_on_overflow']),
    ('s*t', 'c4x', ['-warn_on_overflow']), ('h*t', 'c4', ['-warn_on_overflow']),
    ('xm*t', 'm', []), ('xa1*t', 'a1', []), ('xa2*t', 'a2', []),
    ('xd*t', 'c2', []), ('xc3*t', 'c3', []),
]


def regression_rule(name):
    """Return (level, flags) for a regression test file name, or None."""
    for pattern, level, flags in REGRESSION_RULES:
        if fnmatch.fnmatch(name, pattern):
            return level, flags
    return None
