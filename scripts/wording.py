#!/usr/bin/env python3
"""Find a verb whose object is a negative in a prose stream.

scripts/check-doc-prose.sh runs this for its negative-object check. The prose
rule puts a negation on the verb, not on its object: write "doesn't accept
options" or "didn't change anything", not "accepts no options" or "changed
nothing". A negative subject reads naturally ("Nothing changes", "No test
ran") and is never flagged, and neither are the idioms in which a common verb
takes a negative object ("has no effect", "do nothing"); see ALLOWED_WORDS.

Standard input is a prose stream, one "path:line:text" line for each line of
prose, as check-doc-prose.sh extracts it. Lines with consecutive numbers in the
same file are read as one passage, so a phrase that wraps onto the next line is
still found. Each hit is printed as "path:line:text", where the text is the
line the verb is on, joined with the next line when the phrase wraps, followed
by the suggested rewrite where the verb form makes one simple.

A hit is a word from FLAGGED_VERBS followed, across whitespace alone, by "no",
"nothing", "nobody", "none", or "no one", in any case. These are passed over:

- a verb form after a determiner ("a", "the", "every", ...), which makes it a
  noun: "a change nobody made";
- "no" followed by "longer", "matter", "later", "earlier", "sooner",
  "faster", or "slower", which is an adverb rather than an object.

Usage: scripts/check-doc-prose.sh ... | python3 scripts/wording.py
"""

import bisect
import re
import sys
from typing import NamedTuple

# The verbs the check flags, one family per string: base form, third person
# singular, simple past, past participle, and present participle. A word that is
# also a noun ("changes", "records", "names") is flagged anyway: a determiner
# before it already marks the plain noun use, and a rewrite costs less than a
# missed hit. Linking verbs are flagged only where the phrase reads as a
# negative object, as "becomes" can.
FLAGGED_VERBS = (
    "accept accepts accepted accepted accepting",
    "accompany accompanies accompanied accompanied accompanying",
    "activate activates activated activated activating",
    "add adds added added adding",
    "affect affects affected affected affecting",
    "allow allows allowed allowed allowing",
    "alter alters altered altered altering",
    "answer answers answered answered answering",
    "apply applies applied applied applying",
    "ask asks asked asked asking",
    "assert asserts asserted asserted asserting",
    "attach attaches attached attached attaching",
    "become becomes became become becoming",
    "bring brings brought brought bringing",
    "build builds built built building",
    "buy buys bought bought buying",
    "call calls called called calling",
    "cap caps capped capped capping",
    "carry carries carried carried carrying",
    "catch catches caught caught catching",
    "cause causes caused caused causing",
    "change changes changed changed changing",
    "charge charges charged charged charging",
    "check checks checked checked checking",
    "choose chooses chose chosen choosing",
    "cite cites cited cited citing",
    "claim claims claimed claimed claiming",
    "clear clears cleared cleared clearing",
    "close closes closed closed closing",
    "collect collects collected collected collecting",
    "confirm confirms confirmed confirmed confirming",
    "connect connects connected connected connecting",
    "contact contacts contacted contacted contacting",
    "contain contains contained contained containing",
    "contribute contributes contributed contributed contributing",
    "convert converts converted converted converting",
    "copy copies copied copied copying",
    "cost costs cost cost costing",
    "count counts counted counted counting",
    "cover covers covered covered covering",
    "create creates created created creating",
    "declare declares declared declared declaring",
    "delete deletes deleted deleted deleting",
    "deliver delivers delivered delivered delivering",
    "draw draws drew drawn drawing",
    "drop drops dropped dropped dropping",
    "duplicate duplicates duplicated duplicated duplicating",
    "earn earns earned earned earning",
    "email emails emailed emailed emailing",
    "emit emits emitted emitted emitting",
    "end ends ended ended ending",
    "enter enters entered entered entering",
    "exclude excludes excluded excluded excluding",
    "exempt exempts exempted exempted exempting",
    "expand expands expanded expanded expanding",
    "expect expects expected expected expecting",
    "expire expires expired expired expiring",
    "export exports exported exported exporting",
    "expose exposes exposed exposed exposing",
    "extend extends extended extended extending",
    "fail fails failed failed failing",
    "fetch fetches fetched fetched fetching",
    "file files filed filed filing",
    "fill fills filled filled filling",
    "filter filters filtered filtered filtering",
    "find finds found found finding",
    "finish finishes finished finished finishing",
    "fit fits fitted fitted fitting",
    "flag flags flagged flagged flagging",
    "flip flips flipped flipped flipping",
    "follow follows followed followed following",
    "generate generates generated generated generating",
    "get gets got gotten getting",
    "give gives gave given giving",
    "go goes went gone going",
    "grant grants granted granted granting",
    "hear hears heard heard hearing",
    "help helps helped helped helping",
    "hide hides hid hidden hiding",
    "hold holds held held holding",
    "identify identifies identified identified identifying",
    "import imports imported imported importing",
    "include includes included included including",
    "inject injects injected injected injecting",
    "install installs installed installed installing",
    "involve involves involved involved involving",
    "issue issues issued issued issuing",
    "join joins joined joined joining",
    "keep keeps kept kept keeping",
    "launch launches launched launched launching",
    "lay lays laid laid laying",
    "lead leads led led leading",
    "learn learns learned learned learning",
    "leave leaves left left leaving",
    "let lets let let letting",
    "limit limits limited limited limiting",
    "link links linked linked linking",
    "list lists listed listed listing",
    "load loads loaded loaded loading",
    "log logs logged logged logging",
    "look looks looked looked looking",
    "lose loses lost lost losing",
    "mail mails mailed mailed mailing",
    "manage manages managed managed managing",
    "mark marks marked marked marking",
    "match matches matched matched matching",
    "mean means meant meant meaning",
    "mention mentions mentioned mentioned mentioning",
    "move moves moved moved moving",
    "name names named named naming",
    "narrow narrows narrowed narrowed narrowing",
    "need needs needed needed needing",
    "obtain obtains obtained obtained obtaining",
    "offer offers offered offered offering",
    "open opens opened opened opening",
    "orchestrate orchestrates orchestrated orchestrated orchestrating",
    "order orders ordered ordered ordering",
    "outlive outlives outlived outlived outliving",
    "owe owes owed owed owing",
    "paint paints painted painted painting",
    "pass passes passed passed passing",
    "pay pays paid paid paying",
    "permit permits permitted permitted permitting",
    "pick picks picked picked picking",
    "pin pins pinned pinned pinning",
    "play plays played played playing",
    "post posts posted posted posting",
    "print prints printed printed printing",
    "process processes processed processed processing",
    "produce produces produced produced producing",
    "promise promises promised promised promising",
    "protect protects protected protected protecting",
    "prove proves proved proven proving",
    "provide provides provided provided providing",
    "publish publishes published published publishing",
    "put puts put put putting",
    "queue queues queued queued queuing",
    "raise raises raised raised raising",
    "reach reaches reached reached reaching",
    "reactivate reactivates reactivated reactivated reactivating",
    "read reads read read reading",
    "receive receives received received receiving",
    "record records recorded recorded recording",
    "refund refunds refunded refunded refunding",
    "refuse refuses refused refused refusing",
    "remove removes removed removed removing",
    "render renders rendered rendered rendering",
    "renew renews renewed renewed renewing",
    "report reports reported reported reporting",
    "require requires required required requiring",
    "reset resets reset reset resetting",
    "resemble resembles resembled resembled resembling",
    "restore restores restored restored restoring",
    "restrict restricts restricted restricted restricting",
    "retain retains retained retained retaining",
    "return returns returned returned returning",
    "reveal reveals revealed revealed revealing",
    "rewrite rewrites rewrote rewritten rewriting",
    "run runs ran run running",
    "save saves saved saved saving",
    "say says said said saying",
    "schedule schedules scheduled scheduled scheduling",
    "see sees saw seen seeing",
    "seed seeds seeded seeded seeding",
    "select selects selected selected selecting",
    "sell sells sold sold selling",
    "send sends sent sent sending",
    "serve serves served served serving",
    "set sets set set setting",
    "settle settles settled settled settling",
    "share shares shared shared sharing",
    "ship ships shipped shipped shipping",
    "show shows showed shown showing",
    "sign signs signed signed signing",
    "skip skips skipped skipped skipping",
    "spend spends spent spent spending",
    "stack stacks stacked stacked stacking",
    "stamp stamps stamped stamped stamping",
    "start starts started started starting",
    "store stores stored stored storing",
    "submit submits submitted submitted submitting",
    "supply supplies supplied supplied supplying",
    "tell tells told told telling",
    "terminate terminates terminated terminated terminating",
    "touch touches touched touched touching",
    "transfer transfers transferred transferred transferring",
    "trigger triggers triggered triggered triggering",
    "trust trusts trusted trusted trusting",
    "unaffiliate unaffiliates unaffiliated unaffiliated unaffiliating",
    "undo undoes undid undone undoing",
    "unsubscribe unsubscribes unsubscribed unsubscribed unsubscribing",
    "update updates updated updated updating",
    "use uses used used using",
    "verify verifies verified verified verifying",
    "want wants wanted wanted wanting",
    "work works worked worked working",
    "write writes wrote written writing",
    "yield yields yielded yielded yielding",
)

# The words found before a negative that are deliberately left alone, one group
# per string. Linking and existential verbs ("there is no partner") and modals
# ("may no longer") state a fact, not an action on an object. A few common verbs
# take a negative object idiomatically ("has no effect", "do nothing", "knows
# nothing", "makes no sense", "take no responsibility"); that is good English,
# and the check is for the stilted forms ("accepts no options", "changed
# nothing"). Prepositions ("with no phantoms", "for no one"), conjunctions,
# relative words, and adverbs introduce a negative subject or a noun phrase. A
# pronoun or an article puts the verb out of reach. A word in neither tuple is
# left alone too: this one records the decisions.
ALLOWED_WORDS = (
    # Linking and existential verbs.
    "am are be been being exist existed exists is remain remained remains seem seemed",
    "seems stay stayed stays was were",
    # Modals.
    "can could may might must shall should will would",
    # Verbs whose negative object is idiomatic.
    "do does did done doing have has had having know knows knew known knowing",
    "make makes made made making take takes took taken taking",
    # Prepositions.
    "about after against at before by during for from in into of on onto over per plus",
    "since than through to toward towards under until upon via with within without",
    # Conjunctions, relative words, and adverbs.
    "almost alone although and because but deliberately else here if nor once only or",
    "otherwise possibly simply so that then there though when where whereas whether",
    "which while who whom why yet",
    # Pronouns, articles, and the rest.
    "it not sure the them true you",
)

#: Every flagged verb form, lower case.
FLAGGED_FORMS = frozenset(form for family in FLAGGED_VERBS for form in family.split())

#: A word before the verb form that makes it a noun: "a change nobody made".
DETERMINERS = frozenset(
    "a an any each every her his its my our some the their whose your".split()
)

#: A word after "no" that makes it an adverb rather than an object: "no longer".
NOT_OBJECTS = frozenset("earlier faster later longer matter slower sooner".split())

#: A word after "no" that takes "any" in the rewrite whatever its number.
ANY_BEFORE = frozenset("further less more other such".split())

#: The negative's positive counterpart in a rewrite; "no" depends on the next word.
POSITIVES = {"nothing": "anything", "nobody": "anybody", "none": "any"}

#: The perfect auxiliaries a past participle is rewritten with.
PERFECT_AUXILIARIES = frozenset(("had", "has", "have"))

_SEPARATOR = r"(?:[ \t]+|[ \t]*\n[ \t]*)"

#: The apostrophes that join a contraction into one word: straight and typographic.
_APOSTROPHES = "'" + chr(0x2019)

# A lookahead, so that every word start is tried and one candidate never hides
# the next.
_CANDIDATE = re.compile(
    rf"(?<![\w{_APOSTROPHES}])(?=(?P<verb>[A-Za-z]+)"
    + _SEPARATOR
    + rf"(?P<negative>nothing|nobody|none|no)(?![\w{_APOSTROPHES}-]))",
    re.IGNORECASE,
)

_NEXT_WORD = re.compile(_SEPARATOR + r"([A-Za-z]+)")

_PREVIOUS_WORD = re.compile(rf"(?<![\w{_APOSTROPHES}])([A-Za-z]+)" + _SEPARATOR + r"\Z")


class Family(NamedTuple):
    """The five forms of one flagged verb, in FLAGGED_VERBS order."""

    base: str
    third: str
    past: str
    participle: str
    present_participle: str


FAMILY_OF = {form: Family(*line.split()) for line in FLAGGED_VERBS for form in line.split()}


def find_hits(path, numbered):
    """Yield "path:line:text" for each hit in one file's numbered prose lines."""
    # Consecutive lines form one passage; a gap (code, a blank line, a skipped
    # block) is two line breaks, which no separator crosses.
    text = ""
    starts = []
    numbers = []
    previous_number = None
    for number, line in numbered:
        if previous_number is not None:
            text += "\n" if number == previous_number + 1 else "\n\n"
        starts.append(len(text))
        numbers.append(number)
        text += line
        previous_number = number
    for match in _CANDIDATE.finditer(text):
        verb, negative = match["verb"], match["negative"]
        if verb.lower() not in FLAGGED_FORMS:
            continue
        next_match = _NEXT_WORD.match(text, match.end("negative"))
        next_word = next_match[1] if next_match is not None else None
        if negative.lower() == "no" and next_word is not None and next_word.lower() in NOT_OBJECTS:
            continue
        verb_start = match.start("verb")
        previous_match = _PREVIOUS_WORD.search(text, max(0, verb_start - 80), verb_start)
        previous = previous_match[1] if previous_match is not None else None
        if previous is not None and previous.lower() in DETERMINERS:
            continue
        first = bisect.bisect_right(starts, verb_start) - 1
        last = bisect.bisect_right(starts, match.start("negative")) - 1
        lines = text[starts[first]:].split("\n")[: last - first + 1]
        shown = " ".join(line.strip() for line in lines)
        rewrite = suggestion(verb, negative, next_word=next_word, previous=previous)
        if rewrite is not None:
            shown += f"  [{rewrite}]"
        yield f"{path}:{numbers[first]}:{shown}"


def suggestion(verb, negative, *, next_word, previous):
    """The rewrite of "verb negative" with the negation on the verb, if simple.

    A third-person present form takes "doesn't", a simple past "didn't", and a
    past participle after "has", "have", or "had" that auxiliary's negative.
    Returns None for any other form, whose rewrite depends on the words around it.
    """
    family = FAMILY_OF[verb.lower()]
    form = verb.lower()
    phrase, positive = objects(negative, next_word)
    if previous is not None and previous.lower() in PERFECT_AUXILIARIES and form == family.participle:
        negated = like(previous, f"{previous.lower()}n't")
        return f"{previous} {verb} {phrase} -> {negated} {form} {positive}"
    if form == family.third:
        auxiliary = "doesn't"
    elif form == family.past and family.past != family.base:
        auxiliary = "didn't"
    else:
        return None
    return f"{verb} {phrase} -> {like(verb, auxiliary)} {family.base} {positive}"


def objects(negative, next_word):
    """The negative object as written, and its positive rewrite."""
    lower = negative.lower()
    if lower in POSITIVES:
        return negative, POSITIVES[lower]
    if next_word is None:
        return negative, "any"
    phrase = f"{negative} {next_word}"
    word = next_word.lower()
    if word == "one":
        return phrase, "anyone"
    if word in ANY_BEFORE or (word.endswith("s") and not word.endswith("ss")):
        return phrase, f"any {next_word}"
    article = "an" if word[0] in "aeiou" else "a"
    return phrase, f"{article} {next_word}"


def like(model, word):
    """word capitalized when model is, so a rewrite keeps its sentence case."""
    return word[0].upper() + word[1:] if model[0].isupper() else word


def main():
    by_path = {}
    for raw in sys.stdin:
        parts = raw.rstrip("\n").split(":", 2)
        if len(parts) != 3 or not parts[1].isdigit():
            continue
        by_path.setdefault(parts[0], []).append((int(parts[1]), parts[2]))
    for path, numbered in by_path.items():
        for hit in find_hits(path, numbered):
            print(hit)


if __name__ == "__main__":
    main()
