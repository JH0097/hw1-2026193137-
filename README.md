# 과제 1 - Compare Sorting (C17)

- 이름 / 학번: 최재혁 / 2026193137
- 저장소: https://github.com/JH0097/hw1-2026193137-
- 비교 대상: 삽입 정렬, 병합 정렬, 힙 정렬

정렬 구현, 입력 생성, 시간 측정, 비교·이동 계측, 정확성 테스트는 **모두 C**입니다.
Python은 C 실행 파일을 호출하고 결과 CSV 요약·보고서를 만드는 보조 도구에만 사용합니다.

## 빠른 실행

교수자 실습 템플릿의 Codespaces/Linux 환경에서 저장소 루트 터미널을 엽니다.
C17 컴파일러(GCC 또는 Clang), make, Python 3이 필요합니다.
정렬·테스트 및 CSV 요약에 외부 라이브러리는 필요하지 않습니다.

```sh
make test
make bench
```

- `make test`: 계측용·시간용 두 빌드에서 정확성 검사
- `make bench`: C 실행 파일 2개 실행 후 results/에 CSV 및 실행 환경 저장
- `make run`: 시간 측정용 프로그램만 실행하고 CSV를 표준 출력으로 표시
- `make sanitize`: 지원되는 GCC/Clang에서 AddressSanitizer/UBSan 검사

본 결과는 macOS arm64에서 Zig 0.15.2의 Clang 기반 C 컴파일러로 실제 실행했습니다.
Zig 언어로 작성된 정렬 코드는 없으며, 제출 코드는 GCC/Clang에서도 빌드할 수 있게 작성했습니다.
GCC/Clang에서의 별도 실행 여부와 실제 측정 환경은 results/environment.json을 기준으로 확인하세요.

## make 없이 C 컴파일하기

```sh
mkdir -p build
cc -Isrc -std=c17 -O2 -Wall -Wextra -Wpedantic tests/test_sort.c src/sort.c src/input.c -o build/test-plain.out
cc -Isrc -std=c17 -O2 -Wall -Wextra -Wpedantic -DCOUNT_STATS tests/test_sort.c src/sort.c src/input.c -o build/test.out
./build/test-plain.out
./build/test.out
cc -Isrc -std=c17 -O2 -Wall -Wextra -Wpedantic src/main.c src/sort.c src/input.c -o build/bench.out
cc -Isrc -std=c17 -O2 -Wall -Wextra -Wpedantic -DCOUNT_STATS src/main.c src/sort.c src/input.c -o build/stats.out
python3 tools/run_benchmark.py --compiler cc
```

정렬 자체는 표준 C를 사용하며, 시간 측정은 macOS/Linux에서 지원하는 POSIX clock_gettime을 사용합니다.
Windows에서는 교수자 Codespaces나 WSL/Linux 환경을 이용하세요.

## 파일 구성

- `src/sort.h`, `src/sort.c`: 공통 인터페이스와 세 정렬
- `src/input.h`, `src/input.c`: 입력 생성, 정렬·원소 보존·안정성 검사
- `src/main.c`: C 벤치마크, 예열·호출 순서 순환·배치 측정
- `tests/test_sort.c`: qsort 대조, 경계값, 전수 검사, 안정성 반례, 계수 수식 검사
- `tools/run_benchmark.py`: C 프로그램 호출, 결과 합치기, 중앙값·범위 요약
- `results/timing.csv`: 525개 시간 측정 배치 원본 (각 배치 16회 정렬)
- `results/counts.csv`: 별도 525회 C 연산 계측 원본
- `results/raw.csv`: 동일 시드·결과 해시 검증 후 합친 525행
- `results/summary.csv`: 75개 조건의 중앙값·최소·최대
- `results/environment.json`: 컴파일러, 옵션, 운영체제, 실행 시각, C 소스 해시
- `results/tests.txt`: 실제 테스트 실행 결과
- `report/REPORT.pdf`, `report/REPORT.md`: 보고서 및 읽기용 원본
- `report/AI_LEARNING.md`: 힙 정렬 AI 학습 자료
- `report/student.json`: 이름·학번·저장소 URL
- `tools/build_report.py`: 측정 결과로 PDF 및 Markdown 생성
- `tools/fonts/`: Nanum Gothic 및 OFL 라이선스

## 측정 조건

- 크기: 1,000 / 2,000 / 4,000 / 8,000 / 16,000
- 형태: 무작위 순열 / 오름차순 / 역순 / 0~15 중복 / n/100회 인접 교환
- 고정 기본 시드: 20260930; xorshift32와 거절 샘플링 사용
- 조건별 7회 배치, 각 배치에서 16개의 같은 입력 복사본을 정렬
- 입력 생성·복사·검증은 측정 밖에서 수행; 총 시간을 16으로 나눠 정렬 1회 평균 계산
- 7개 배치 평균의 중앙값을 보고; 모든 원본과 최소·최대 보존
- 계측 코드는 C 전처리기로 시간용 빌드에서 제거하고 별도 빌드에서 실행
- 이동: 코드상 Record 대입 횟수, 임시 저장 포함, 교환 3회
- 추가 공간: 병합 보조 배열 할당 바이트와 이론적 공간 복잡도; 프로세스 RSS 실측 아님

## 보고서 다시 만들기

실험을 재실행하면 results/가 새 결과로 갱신됩니다. 보고서와 일치하도록 PDF도 다시 만들고,
results/와 report/를 함께 커밋하세요.

```sh
python3 -m pip install -r tools/report-requirements.txt
python3 tools/build_report.py
```

PDF 생성에만 reportlab이 필요합니다. Python 보조 도구는 정렬 시간 구간에 포함되지 않습니다.
문장 수정은 `tools/build_report.py`에서 합니다. REPORT.md를 직접 고쳐도 PDF에 자동 반영되지 않습니다.

## 참고 및 AI 사용

AI가 코드·검사·실험 실행·보고서·힙 정렬 학습 자료 작성을 보조했습니다.
수치는 실제 C 프로그램 실행 결과이며, 학생이 직접 실행했다는 의미는 아닙니다.
학습 자료는 AI가 작성한 설명형 문답입니다. 실제 대화 원문을 가장한 기록은 아닙니다.

- 강의: https://github.com/lec-algorithm/lecture
- 실습 코드: https://github.com/lec-algorithm/algorithm-code
- 샘플: https://github.com/lec-algorithm/hw1-sample-2026
- 실습 템플릿: https://github.com/lec-algorithm/algorithm-env
- 정렬 비교표: https://en.wikipedia.org/wiki/Sorting_algorithm#Comparison_of_algorithms
- 힙 정렬 이론: https://algs4.cs.princeton.edu/24pq/
