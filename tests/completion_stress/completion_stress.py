#!/usr/bin/env python3
"""Differential stress tests for Sdtty's command completion.

Builds Sdtty from two git refs, types the same input into both, and reports
every session whose output differs.  See README.md.
"""

import argparse
import os
import re
import resource
import shutil
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor

import cases
import fuzzgen

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
BUILDS = os.path.join(HERE, 'builds')
OUT = os.path.join(HERE, 'out')
DEFAULT_REGRESSION = os.path.join(os.path.dirname(REPO), 'sd-regression', 'regression_tests')

DEL = '\x7f' * 460   # erases the line: more than the 400-character input limit
EXIT = 'exit the program\ny\n'
LEVELS = ['m', 'a1', 'a2', 'c1', 'c2', 'c3', 'c4', 'c4x']
ASAN_FLAGS = '-O1 -g -fsanitize=address,undefined -fsanitize-recover=address -fno-omit-frame-pointer'

# In -diagnostic mode, Sdtty exits when Tab, space, or Enter can't complete
# the line.  The tests press those keys on lines that often can't be
# completed, so each build carries this patch: with SD_PROBE set in the
# environment, Sdtty rings the bell instead, as it does outside -diagnostic.
PROBE_RE = re.compile(r'(if \(ui_options\.diagnostic_mode)\)(\s*\n\s*goto diagnostic_error;)')
PROBE_SUB = r'\1 && !getenv("SD_PROBE"))\2'


# ----------------------------------------------------------------------------
# Builds

class Build:
    def __init__(self, name):
        self.dir = name if os.sep in name else os.path.join(BUILDS, name)
        self.name = os.path.basename(os.path.normpath(self.dir))
        self.src = os.path.join(self.dir, 'src')
        if not os.path.exists(os.path.join(self.src, 'sdtty')):
            sys.exit(f'{self.dir} has no src/sdtty; build it first with "build {name} <ref>"')
        self.asan = os.path.exists(os.path.join(self.dir, 'asan'))


def patch_probe(path):
    with open(path, encoding='latin-1') as f:
        text = f.read()
    text, n = PROBE_RE.subn(PROBE_SUB, text)
    if n == 0:
        sys.exit(f'{path}: found nowhere to apply the SD_PROBE patch')
    with open(path, 'w', encoding='latin-1') as f:
        f.write(text)
    return n


def makefile_defs(src, makefile):
    with open(os.path.join(src, makefile)) as f:
        for ln in f:
            m = re.match(r'DEFS\s*=\s*(.*)', ln)
            if m:
                return ' '.join(w for w in m.group(1).split() if w != '-static')
    return ''


def cmd_build(a):
    dest = os.path.join(BUILDS, a.name)
    if os.path.exists(dest):
        if not a.force:
            sys.exit(f'{dest} exists; pass --force to replace it')
        shutil.rmtree(dest)
    os.makedirs(dest)
    src = os.path.join(dest, 'src')
    if a.ref is None:
        skip = shutil.ignore_patterns('*.o', 'sdtty', 'mkcalls', 'sd_calls.dat', 'sd_calls.*cache')
        shutil.copytree(os.path.join(REPO, 'src'), src, ignore=skip)
        desc = 'working tree'
    else:
        sha = subprocess.run(['git', 'rev-parse', '--verify', a.ref + '^{commit}'], cwd=REPO,
                             capture_output=True, text=True)
        if sha.returncode != 0:
            shutil.rmtree(dest)
            sys.exit(f'unknown git ref {a.ref!r}')
        desc = f'{a.ref} ({sha.stdout.strip()})'
        archive = subprocess.Popen(['git', 'archive', sha.stdout.strip(), 'src'], cwd=REPO,
                                   stdout=subprocess.PIPE)
        subprocess.run(['tar', '-x', '-C', dest], stdin=archive.stdout, check=True)
        archive.wait()
    n = patch_probe(os.path.join(src, 'sdui-tty.cpp'))
    make = ['make', '-f', a.makefile]
    if a.asan:
        make += [f'CDEBUGFLAGS={ASAN_FLAGS}', f'DEFS={makefile_defs(src, a.makefile)}']
        open(os.path.join(dest, 'asan'), 'w').close()
    print(f'building {a.name} from {desc}; SD_PROBE patch applied in {n} places', flush=True)
    with open(os.path.join(dest, 'build.log'), 'w') as log:
        rc = subprocess.run(make + [f'-j{a.jobs}'], cwd=src, stdout=log, stderr=subprocess.STDOUT).returncode
    if rc != 0:
        sys.exit(f'build failed; see {dest}/build.log')
    with open(os.path.join(dest, 'source.txt'), 'w') as f:
        f.write(desc + ('\nAddressSanitizer + UBSan\n' if a.asan else '\n'))
    # Make the database caches now, so parallel sessions don't race to write them.
    for lvl in LEVELS:
        subprocess.run(['./sdtty', lvl, '-db', 'sd_calls.dat', '-diagnostic', '-no_console',
                        '-no_graphics', '-no_color'], cwd=src, input=EXIT.encode(),
                       capture_output=True, timeout=600, env=sdtty_env(a.asan, None))
    print(f'built {dest}/src/sdtty')


# ----------------------------------------------------------------------------
# Running sessions

def sdtty_env(asan, logdir):
    env = dict(os.environ, SD_PROBE='1')
    if asan:
        log = f':log_path={logdir}/asan' if logdir else ''
        env['ASAN_OPTIONS'] = 'detect_leaks=0:halt_on_error=0' + log
        env['UBSAN_OPTIONS'] = 'print_stacktrace=1' + (f':log_path={logdir}/ubsan' if logdir else '')
    return env


def normalize(text):
    # Drop the banner above the version line; it says whether the cache was hit.
    i = text.find('\nVersion')
    return text[i:] if i >= 0 else text


def run_sdtty(build, level, flags, stdin, timeout, memory_gb, logdir=None):
    """Run one session.  Returns (rc, stdout, stderr); rc is 'TIMEOUT' on timeout."""
    cmd = ['./sdtty', level, '-db', 'sd_calls.dat', '-diagnostic', '-no_console', '-no_graphics',
           '-no_color', '-old_style_filename'] + flags
    limit = None
    if memory_gb and not build.asan:   # AddressSanitizer needs a huge address space
        def limit():
            lim = int(memory_gb * (1 << 30))
            resource.setrlimit(resource.RLIMIT_AS, (lim, lim))
    try:
        p = subprocess.run(cmd, cwd=build.src, input=stdin.encode('latin-1'), capture_output=True,
                           timeout=timeout, env=sdtty_env(build.asan, logdir), preexec_fn=limit)
        return p.returncode, normalize(p.stdout.decode('latin-1')), p.stderr.decode('latin-1')
    except subprocess.TimeoutExpired as e:
        return ('TIMEOUT', normalize((e.stdout or b'').decode('latin-1')),
                (e.stderr or b'').decode('latin-1'))


def classify(ra, rb):
    """Compare two (rc, stdout) results."""
    if ra == rb:
        return 'same'
    # A session that timed out stops partway; count it as matching if what it
    # printed is the start of what the other build printed.
    if ra[0] == 'TIMEOUT' and rb[1].startswith(ra[1]):
        return 'timeout-a'
    if rb[0] == 'TIMEOUT' and ra[1].startswith(rb[1]):
        return 'timeout-b'
    return 'differ'


def safe_name(idx, key):
    text = re.sub(r'[^A-Za-z0-9_.,\'-]+', '~', ' '.join(str(k) for k in key))
    return f'{idx:05d}_{text[:120]}'


def compare_jobs(a, jobs):
    """Run each job on both builds.  A job is (key, level, flags, stdin, timeout)."""
    A, B = Build(a.build_a), Build(a.build_b)
    if A.name == B.name:
        sys.exit('the two builds have the same name')
    if a.limit:
        jobs = jobs[:a.limit]
    out = a.out or os.path.join(OUT, a.command)
    os.makedirs(out, exist_ok=True)
    print(f'{a.command}: {len(jobs)} sessions, {A.name} vs {B.name}; differences go to {out}', flush=True)

    def one(job):
        key, level, flags, stdin, timeout = job
        res = {b.name: run_sdtty(b, level, flags, stdin, timeout, a.memory_gb) for b in (A, B)}
        return job, res

    counts = {'same': 0, 'differ': 0, 'timeout-a': 0, 'timeout-b': 0}
    t0 = time.time()
    with ThreadPoolExecutor(a.workers) as ex:
        for idx, (job, res) in enumerate(ex.map(one, jobs)):
            key, stdin = job[0], job[3]
            ra, rb = res[A.name][:2], res[B.name][:2]
            kind = classify(ra, rb)
            counts[kind] += 1
            rcs = f'{A.name} rc={ra[0]}, {B.name} rc={rb[0]}'
            if kind == 'differ':
                tag = safe_name(idx, key)
                with open(os.path.join(out, tag + '.input'), 'w', encoding='latin-1') as f:
                    f.write(stdin)
                for b in (A, B):
                    rc, stdout, stderr = res[b.name]
                    with open(os.path.join(out, f'{tag}.{b.name}'), 'w', encoding='latin-1') as f:
                        f.write(f'rc={rc}\n{stdout}')
                print(f'DIFFER {key}: {rcs} -> {tag}.*', flush=True)
            elif kind != 'same':
                print(f'TIMEOUT {key}: {rcs}', flush=True)
            if kind != 'differ' and (ra[0] not in (0, 'TIMEOUT') or rb[0] not in (0, 'TIMEOUT')):
                print(f'EXIT {key}: {rcs} (same output on both)', flush=True)
    print(f'done in {time.time() - t0:.0f} s: {len(jobs)} sessions, {counts["same"]} same, '
          f'{counts["differ"]} differ, {counts["timeout-a"]} timed out on {A.name} and '
          f'{counts["timeout-b"]} on {B.name} with matching output up to that point', flush=True)
    return 1 if counts['differ'] else 0


def key_probes(line, every_prefix):
    """Tab and space after each prefix (or after each ']'), and '?' after each ']'."""
    ends = [k for k in range(1, len(line) + 1) if line[k - 1] == ']']
    tab_at = range(1, len(line)) if every_prefix else \
        sorted(set(ends) | {k + 1 for k in ends if k < len(line)} | {len(line)})
    parts = []
    for k in tab_at:
        parts += [line[:k] + '\t' + DEL, line[:k] + ' ' + DEL]
    parts += [line[:k] + '?' + DEL for k in ends]
    return ''.join(parts)


def enter_points(line):
    """Prefix lengths at and just after each ']'."""
    return sorted({k for k in range(1, len(line) + 1) if line[k - 1] == ']'} |
                  {k + 1 for k in range(1, len(line)) if line[k - 1] == ']'})


def regression_files(a):
    d = a.regression_dir
    if not os.path.isdir(d):
        sys.exit(f'no regression tests at {d}; pass --regression-dir')
    out = []
    for name in sorted(os.listdir(d)):
        rule = cases.regression_rule(name)
        if not rule or name.endswith(('.tmp', '.ref')):
            continue
        with open(os.path.join(d, name), encoding='latin-1') as f:
            lines = f.read().split('\n')
        if lines and lines[-1] == '':
            lines.pop()
        if any('[' in ln for ln in lines):
            out.append((name, rule, lines))
    return out


def cmd_suite_keys(a):
    jobs = []
    for name, (level, flags), lines in regression_files(a):
        parts = []
        for ln in lines:
            if '[' in ln:
                parts.append(key_probes(ln, every_prefix=True))
            parts.append(ln + '\n')
        jobs.append(((name,), level, flags, ''.join(parts), a.timeout or 3600))
    return compare_jobs(a, jobs)


def cmd_suite_enter(a):
    jobs = []
    for name, (level, flags), lines in regression_files(a):
        for j, ln in enumerate(lines):
            if '[' in ln:
                pre = ''.join(x + '\n' for x in lines[:j])
                for k in enter_points(ln):
                    stdin = pre + ln[:k] + '\n' + DEL + EXIT
                    jobs.append(((name, j + 1, k), level, flags, stdin, a.timeout or 600))
    return compare_jobs(a, jobs)


def cmd_lines_keys(a):
    jobs = []
    for s in cases.LINE_SETUPS:
        for ln in cases.LINES:
            stdin = cases.SETUPS[s] + key_probes(ln, every_prefix=True) + ln + '\n' + EXIT
            jobs.append(((s, ln), a.level, [], stdin, a.timeout or 1800))
    return compare_jobs(a, jobs)


def cmd_lines_enter(a):
    jobs = []
    for s in cases.LINE_SETUPS:
        for ln in cases.LINES:
            for k in range(1, len(ln) + 1):
                stdin = cases.SETUPS[s] + ln[:k] + '\n' + DEL + EXIT
                jobs.append(((s, ln, k), a.level, [], stdin, a.timeout or 600))
    return compare_jobs(a, jobs)


def cmd_yield(a):
    jobs = []
    for s in cases.YIELD_SETUPS:
        for ln in cases.yield_lines():
            stdin = cases.SETUPS[s] + key_probes(ln, every_prefix=False) + ln + '\n' + DEL + EXIT
            jobs.append(((s, ln), a.level, [], stdin, a.timeout or 900))
    return compare_jobs(a, jobs)


def cmd_speed(a):
    builds = [Build(n) for n in a.builds]
    print(f'Each call typed in {cases.SPEED_SETUP}, with Tab and space after each "]", then Enter.')
    for ln in cases.SPEED_LINES:
        keys = ''.join(ln[:k] + key + DEL for k in enter_points(ln) if ln[k - 1] == ']'
                       for key in '\t ')
        stdin = cases.SETUPS[cases.SPEED_SETUP] + keys + ln + '\n' + EXIT
        outs = {}
        print(ln)
        for b in builds:
            t = time.time()
            rc, stdout, _ = run_sdtty(b, a.level, [], stdin, a.timeout or 900, a.memory_gb)
            outs[b.name] = (rc, stdout)
            print(f'  {b.name:20} {time.time() - t:8.1f} s  rc={rc}', flush=True)
        if len(set(outs.values())) > 1:
            print('  (the builds printed different output)')
    return 0


def cmd_fuzz(a):
    builds = [Build(n) for n in a.builds]
    gen = fuzzgen.Generator(builds[0].src)
    out = a.out or os.path.join(OUT, 'fuzz')
    os.makedirs(out, exist_ok=True)
    seeds = list(range(a.first_seed, a.first_seed + a.sessions))
    names = ', '.join(b.name for b in builds)
    print(f'fuzz: {len(seeds)} sessions from seed {a.first_seed} on {names}; crashes go to {out}',
          flush=True)

    def reports_in(logdir):
        found = []
        for f in sorted(os.listdir(logdir)):
            with open(os.path.join(logdir, f), encoding='latin-1') as fh:
                found += re.findall(r'(SUMMARY: \S+: [^\n]*|runtime error: [^\n]*)', fh.read())
        return found

    # Reports that a sanitizer build prints on every run, before any input.
    startup = {}
    for b in builds:
        if b.asan:
            logdir = tempfile.mkdtemp(prefix=f'{b.name}-startup-', dir=out)
            run_sdtty(b, 'c4x', [], EXIT, 900, 0, logdir)
            startup[b.name] = set(reports_in(logdir))
            shutil.rmtree(logdir)
            if startup[b.name]:
                print(f'ignoring {len(startup[b.name])} report(s) that {b.name} prints at start-up',
                      flush=True)

    def one(seed):
        stdin = gen.session(seed)
        res = {}
        for b in builds:
            logdir = tempfile.mkdtemp(prefix=f'{b.name}-{seed}-', dir=out) if b.asan else None
            rc, stdout, stderr = run_sdtty(b, 'c4x', [], stdin, a.timeout or (900 if b.asan else 180),
                                           a.memory_gb, logdir)
            reports = []
            if logdir:
                reports = [r for r in reports_in(logdir) if r not in startup[b.name]]
                if not reports:
                    shutil.rmtree(logdir)
            if rc == 'TIMEOUT' and stdout.count('XXX') > 20:
                rc = 'EOFLOOP'   # older builds loop forever once input runs out
            res[b.name] = (rc, stderr[-300:], sorted(set(reports)), logdir)
        return seed, stdin, res

    bad = {b.name: 0 for b in builds}
    new = 0
    t0 = time.time()
    with ThreadPoolExecutor(a.workers) as ex:
        for seed, stdin, res in ex.map(one, seeds):
            failed = {n: r for n, r in res.items() if r[0] not in (0, 'TIMEOUT') or r[2]}
            for n, (rc, err, reports, logdir) in failed.items():
                bad[n] += 1
                where = f' (logs in {logdir})' if reports else ''
                print(f'BAD seed={seed} {n}: rc={rc} reports={reports[:3]} stderr={err[-120:]!r}{where}',
                      flush=True)
            for n, r in res.items():
                if r[0] == 'TIMEOUT':
                    print(f'TIMEOUT seed={seed} {n}', flush=True)
            if failed:
                with open(os.path.join(out, f'seed-{seed}.input'), 'w', encoding='latin-1') as f:
                    f.write(stdin)
            if len(builds) == 2 and builds[1].name in failed and builds[0].name not in failed:
                new += 1
                print(f'NEW seed={seed}: fails on {builds[1].name} only', flush=True)
    summary = ', '.join(f'{n} failed {c}' for n, c in bad.items())
    extra = f'; {new} fail on {builds[1].name} only' if len(builds) == 2 else ''
    print(f'done in {time.time() - t0:.0f} s: {len(seeds)} sessions; {summary}{extra}', flush=True)
    return 1 if (new if len(builds) == 2 else sum(bad.values())) else 0


# ----------------------------------------------------------------------------

def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='command', required=True)

    b = sub.add_parser('build', help='build a patched sdtty from a git ref into builds/NAME')
    b.add_argument('name')
    b.add_argument('ref', nargs='?', help='git ref or commit; omit to build the working tree')
    b.add_argument('--asan', action='store_true', help='build with AddressSanitizer and UBSan')
    b.add_argument('--makefile', default='makefile.linux64')
    b.add_argument('--jobs', type=int, default=1, help='parallel make jobs (default: 1)')
    b.add_argument('--force', action='store_true', help='replace an existing build')
    b.set_defaults(func=cmd_build)

    def common(sp, level=True):
        sp.add_argument('--workers', type=int, default=max(1, (os.cpu_count() or 2) // 2),
                        help='sessions to run at once (default: half the CPUs)')
        sp.add_argument('--timeout', type=float, help='seconds per session')
        sp.add_argument('--memory-gb', type=float, default=3,
                        help='address-space limit per sdtty; 0 for none (default: 3)')
        if level:
            sp.add_argument('--level', default='c4x', help='level for the built-in lines (default: c4x)')

    compare = [
        ('suite-keys', cmd_suite_keys, 'Tab, space, and ? on every bracketed regression-suite line'),
        ('suite-enter', cmd_suite_enter, 'Enter at and after each "]" of every regression-suite line'),
        ('lines-keys', cmd_lines_keys, 'Tab, space, and ? on the built-in nested-bracket lines'),
        ('lines-enter', cmd_lines_enter, 'Enter after every prefix of the built-in lines'),
        ('yield', cmd_yield, 'yield_if_ambiguous calls inside nested brackets'),
    ]
    for name, func, help_text in compare:
        sp = sub.add_parser(name, help=help_text)
        sp.add_argument('build_a', help='the reference build, usually the base branch')
        sp.add_argument('build_b', help='the build under test')
        common(sp)
        sp.add_argument('--limit', type=int, help='run only the first N sessions')
        sp.add_argument('--out', help=f'directory for differences (default: out/{name})')
        if name.startswith('suite'):
            sp.add_argument('--regression-dir', default=DEFAULT_REGRESSION,
                            help='sd-regression/regression_tests (default: the sibling checkout)')
        sp.set_defaults(func=func)

    sp = sub.add_parser('speed', help='time the slow example calls on each build')
    sp.add_argument('builds', nargs='+')
    common(sp)
    sp.set_defaults(func=cmd_speed)

    sp = sub.add_parser('fuzz', help='random input; reports crashes and sanitizer errors')
    sp.add_argument('builds', nargs='+', help='one build, or a reference build and the build under test')
    sp.add_argument('--first-seed', type=int, default=1000)
    sp.add_argument('--sessions', type=int, default=1000)
    sp.add_argument('--out', help='directory for failing inputs (default: out/fuzz)')
    common(sp, level=False)
    sp.set_defaults(func=cmd_fuzz)

    a = p.parse_args()
    if getattr(a, 'builds', None) and len(a.builds) > 2 and a.command == 'fuzz':
        p.error('fuzz takes one or two builds')
    sys.exit(a.func(a))


if __name__ == '__main__':
    main()
