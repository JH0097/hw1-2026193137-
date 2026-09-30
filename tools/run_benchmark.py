"""Run already compiled C executables, validate raw results and summarize.
No Python sorting implementation is used in the experiment.
"""
import argparse
import csv
import hashlib
import json
import platform
import shlex
import statistics
import subprocess
from collections import defaultdict
from datetime import datetime
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--compiler',default='cc')
    ap.add_argument('--flags',default='-Isrc -std=c17 -O2 -Wall -Wextra -Wpedantic')
    args=ap.parse_args()
    out=ROOT/'results'; out.mkdir(exist_ok=True)
    started=datetime.now().astimezone().isoformat()
    version=subprocess.run(shlex.split(args.compiler)+['--version'],text=True,capture_output=True,check=True).stdout.strip()
    for binary,filename in [('bench.out','timing.csv'),('stats.out','counts.csv')]:
        with (out/filename).open('w') as f:
            subprocess.run([str(ROOT/'build'/binary)],cwd=ROOT,stdout=f,check=True)
    def read(name):
        with (out/name).open() as f: return list(csv.DictReader(f))
    timing, counts=read('timing.csv'),read('counts.csv')
    key=lambda r:(r['pattern'],r['n'],r['trial'],r['algorithm'])
    bykey={key(r):r for r in counts}
    assert len(timing)==len(counts)==len(bykey)==525
    assert len({key(r) for r in timing})==525
    merged=[]
    for r in timing:
        c=bykey[key(r)]
        for field in ['seed','valid','stable','hash','record_bytes']:
            assert r[field]==c[field], (key(r),field)
        assert r['valid']=='1' and int(r['batch'])==16 and int(c['batch'])==1
        assert float(r['ms'])>0 and float(c['ms'])==0
        if r['algorithm']!='heap': assert r['stable']=='1'
        new=r.copy()
        for field in ['comparisons','moves','buffer_bytes']: new[field]=c[field]
        merged.append(new)
    def save(name,data):
        with (out/name).open('w',newline='') as f:
            w=csv.DictWriter(f,fieldnames=list(data[0])); w.writeheader(); w.writerows(data)
    save('raw.csv',merged)
    groups=defaultdict(list)
    for r in merged: groups[(r['pattern'],int(r['n']),r['algorithm'])].append(r)
    summary=[]
    for (p,n,a),group in groups.items():
        assert len(group)==7
        times=[float(r['ms']) for r in group]
        summary.append(dict(pattern=p,n=n,algorithm=a,median_ms=statistics.median(times),
            min_ms=min(times),max_ms=max(times),
            median_comparisons=statistics.median(int(r['comparisons']) for r in group),
            median_moves=statistics.median(int(r['moves']) for r in group),
            buffer_bytes=int(group[0]['buffer_bytes']),record_bytes=int(group[0]['record_bytes']),
            stable_trials=sum(int(r['stable']) for r in group),trials=len(group)))
    save('summary.csv',summary)
    hashes={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest()
            for folder in ['src','tests'] for p in sorted((ROOT/folder).glob('*')) if p.suffix in ['.c','.h']}
    env=dict(language='C17 + POSIX clock_gettime',started=started,finished=datetime.now().astimezone().isoformat(),
        system=platform.system(),release=platform.release(),machine=platform.machine(),processor=platform.processor(),
        compiler=version,compiler_command=args.compiler,flags=args.flags,stats_flag='-DCOUNT_STATS',
        sizes=[1000,2000,4000,8000,16000],patterns=['random','sorted','reverse','duplicates','nearly'],trials=7,
        base_seed=20260930,prng='xorshift32 + rejection sampling; Fisher-Yates shuffle',batch=16,
        timer='clock_gettime(CLOCK_MONOTONIC), elapsed nanoseconds / batch / 1e6',
        record_bytes=int(merged[0]['record_bytes']),counter_policy='two executables; timing counters removed by C preprocessor',
        source_sha256=hashes)
    (out/'environment.json').write_text(json.dumps(env,ensure_ascii=False,indent=2)+'\n')
    print(f'Validated 525 timing batches (8400 sorts), 525 counter runs, {len(summary)} summaries.')
if __name__=='__main__': main()
