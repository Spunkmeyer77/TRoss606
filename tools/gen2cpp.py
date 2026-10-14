#!/usr/bin/env python3
"""Translate the TRoss 606 gen~ codebox (GenExpr) into a C++ header.
usage: gen2cpp.py gen.genexpr table.txt out.h"""
import re, sys

src = open(sys.argv[1]).read()
table = open(sys.argv[2]).read().split()
out_path = sys.argv[3]

lines = src.split('\n')
main_idx = next(i for i, l in enumerate(lines) if l.strip() == '// MAIN')

params, hists, datas, bufs = [], [], [], []
func_lines, main_lines = [], lines[main_idx + 1:]
for l in lines[:main_idx]:
    m = re.match(r'^Param (\w+)\(([^)]*)\);', l)
    if m: params.append((m.group(1), m.group(2))); continue
    m = re.match(r'^History (\w+)\(([^)]*)\);', l)
    if m: hists.append((m.group(1), m.group(2))); continue
    m = re.match(r'^Data (\w+)\((\d+)\);', l)
    if m: datas.append((m.group(1), int(m.group(2)))); continue
    m = re.match(r'^Buffer (\w+)\("([^"]*)"\);', l)
    if m: bufs.append(m.group(1)); continue
    func_lines.append(l)

members = {n for n, _ in params} | {n for n, _ in hists} | {n for n, _ in datas} | set(bufs)
FUNCS = {'exp', 'sin', 'cos', 'tanh', 'pow', 'sqrt', 'floor', 'ceil', 'abs', 'min', 'max', 'clamp',
         'noise', 'peek', 'poke'}

def num_to_double(s):
    # make every integer literal a double so GenExpr's float maths is kept (1/2 == 0.5)
    return re.sub(r'(?<![\w.])(\d+)(?![\w.])', r'\1.0', s)

def fix_calls(s):
    s = re.sub(r'\bnoise\(\)', 'noise_()', s)
    s = re.sub(r'\bpeek\(', 'peek_(', s)
    s = re.sub(r'\bpoke\(', 'poke_(', s)
    s = re.sub(r'\babs\(', 'std::fabs(', s)
    for f in ('min', 'max', 'clamp'):
        s = re.sub(r'\b%s\(' % f, 'g%s(' % f, s)
    for f in ('exp', 'sin', 'cos', 'tanh', 'pow', 'sqrt', 'floor', 'ceil'):
        s = re.sub(r'(?<![\w:])%s\(' % f, 'std::%s(' % f, s)
    s = re.sub(r'\bsamplerate\b', 'fs_', s)
    return s

def assigned(text):
    return set(re.findall(r'(?<![\w.=!<>])([A-Za-z_]\w*)\s*=(?!=)', text))

# ---- helper functions
helpers = []
ftext = '\n'.join(func_lines)
for m in re.finditer(r'^(\w+)\(([^)]*)\)\s*\{\n(.*?)^\}', ftext, re.S | re.M):
    name, args, body = m.group(1), [a.strip() for a in m.group(2).split(',') if a.strip()], m.group(3)
    locs = sorted(assigned(body) - set(args))
    b = fix_calls(num_to_double(body))
    decl = ('    double %s;\n' % ', '.join(l + ' = 0.0' for l in locs)) if locs else ''
    helpers.append('static inline double %s(%s) {\n%s%s}\n' % (
        name, ', '.join('double ' + a for a in args), decl, b))

# ---- main body
body = '\n'.join(main_lines)
outs = {}
for m in re.finditer(r'^(out\d+)\s*=\s*(.*);', body, re.M):
    outs[m.group(1)] = m.group(2)
body = re.sub(r'^out\d+\s*=.*;\n?', '', body, flags=re.M)
locs = sorted(assigned(body) - members)
body = fix_calls(num_to_double(body))

o = []
o.append('''// GENERATED from the TRoss 606 gen~ codebox by tools/gen2cpp.py - do not edit by hand.
// Drum voices ported from analogcode/606-Inspired-Synth-Drums (MIT, (c) 2026 Matthew Fecher).
#pragma once
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstring>

#if defined (__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wfloat-equal"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wshadow"
 #pragma clang diagnostic ignored "-Wunused-variable"
 #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined (__GNUC__)
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wfloat-equal"
 #pragma GCC diagnostic ignored "-Wsign-conversion"
 #pragma GCC diagnostic ignored "-Wshadow"
 #pragma GCC diagnostic ignored "-Wunused-variable"
 #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#elif defined (_MSC_VER)
 #pragma warning (push)
 #pragma warning (disable: 4100 4189 4244 4456 4457 4458)
#endif

namespace tross606 {

static inline double gmin(double a, double b) { return a < b ? a : b; }
static inline double gmax(double a, double b) { return a > b ? a : b; }
static inline double gclamp(double x, double lo, double hi) { return x < lo ? lo : (x > hi ? hi : x); }

''')
o.extend(helpers)
o.append('''
struct GenVoices {
    // ---- Params (set from the plug-in)
''')
for n, v in params:
    o.append('    double %s = %s;\n' % (n, num_to_double(v) if v else '0.0'))
o.append('\n    // ---- History\n')
for n, v in hists:
    o.append('    double %s = %s;\n' % (n, num_to_double(v)))
o.append('\n    // ---- Data / Buffer\n')
for n, sz in datas:
    o.append('    double %s[%d] = {};\n' % (n, sz))
for n in bufs:
    o.append('    double %s[%d] = {\n' % (n, len(table)))
    for i in range(0, len(table), 8):
        o.append('        ' + ', '.join(table[i:i + 8]) + ',\n')
    o.append('    };\n')
o.append('''
    double fs_ = 44100.0;
    uint32_t rng_ = 0x12345678u;

    inline double noise_() {   // white noise in [-1, 1)
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5;
        return (double) rng_ * (2.0 / 4294967296.0) - 1.0;
    }
    template <std::size_t N> static inline double peek_(const double (&d)[N], double idx) {
        const int i = (int) std::floor(idx);
        return (i >= 0 && i < (int) N) ? d[(std::size_t) i] : 0.0;
    }
    template <std::size_t N> static inline void poke_(double (&d)[N], double v, double idx) {
        const int i = (int) std::floor(idx);
        if (i >= 0 && i < (int) N) d[(std::size_t) i] = v;
    }

    // one sample; returns the mono mix
    inline double tick() {
''')
for i in range(0, len(locs), 6):
    o.append('        double ' + ', '.join(l + ' = 0.0' for l in locs[i:i + 6]) + ';\n')
for l in body.split('\n'):
    o.append('        ' + l + '\n' if l.strip() else '')
o.append('        return %s;\n    }\n};\n\n} // namespace tross606\n' % outs['out1'])
o.append('''
#if defined (__clang__)
 #pragma clang diagnostic pop
#elif defined (__GNUC__)
 #pragma GCC diagnostic pop
#elif defined (_MSC_VER)
 #pragma warning (pop)
#endif
''')
open(out_path, 'w').write(''.join(o))
print('params', len(params), 'hist', len(hists), 'locals', len(locs), 'helpers', len(helpers))
