# Red-Black Tree ↔ 2-3 Tree 동형 (Isomorphism)

> RB tree는 2-3 tree의 binary encoding 결과이다.

---

## 도입 배경: BST → AVL → 2-3 → RB

**BST의 문제**
일반 BST는 입력이 정렬돼 들어오면 skewed 모양이 되어, search/insert/delete가 O(log n)에서 **O(n)으로 퇴화**한다.

**AVL — 사후 복구 패러다임**
이를 해결하려고 AVL 도입. bf(balance factor) 절댓값을 1 이하로 유지해 skewed를 막고, 균형이 깨지면 **회전으로 사후 복구**한다. (회전 자체는 O(1), 삽입당 상수 번이라 비싸지 않음.)

**2-3 트리 — 다른 패러다임 + 실용 동기**
2-3 트리는 "회전이 싫어서"가 아니라 **균형 유지 방식 자체가 다른** 접근이다.
- *패러다임*: leaf에 매달아 사후 복구하는 대신, 노드를 키워(2-노드→3-노드) 흡수하다 넘치면 split해서 **위로 자란다**. 그 결과 모든 leaf의 depth가 항상 동일 — 불균형이 애초에 안 생긴다 (perfect balance).
- *실용 동기*: 노드에 키를 여러 개 담으면 트리 높이가 낮아져, 디스크/메모리에서 **한 번의 접근으로 더 많은 데이터**를 가져온다 → I/O 감소. (B-트리로 일반화, DB 인덱스·파일시스템의 핵심.)

**RB 트리 — 2-3 트리의 이진 인코딩**
2-3 트리는 노드 타입이 2종(2-노드/3-노드)이라 구현이 번잡하다. 이를 **이진 트리 하나 + 색 1비트**로 통일한 것이 RB 트리다. (자세한 대응은 아래.)

---

## 0. 핵심 직관

**`red` 노드 = "나는 부모(black)와 2-3 트리에서 원래 한 노드였다"는 표시.**

RB 트리는 노드당 키 1개밖에 못 담는 이진 트리다.
그런데 2-3 트리의 3-노드는 키가 2개다.
→ 키 2개를 노드 2개로 쪼개되, "원래 한몸이었다"는 사실을 **색(red)**으로 기록한 것이 RB 트리다.

이 한 문장에서 RB의 모든 규칙이 파생된다.

---

## 1. 노드 대응

### 2-노드 = black 단독
```
2-3:   [x]            RB:   (x)black
      /   \                /        \
    <x    >x            <x          >x
```
키 1개라 쪼갤 필요 없음. red 짝이 없음.

### 3-노드 = black + red 자식 1개
```
2-3:    [a | b]            RB:      (b)black
       /   |   \                   /        \
      L    M    R              (a)red        R
                               /    \
                              L      M
```
순서 검증: `L < a < M < b < R` (양쪽 동일)
- b = black 부모, a = b의 **왼쪽 red 자식** (left-leaning 기준)
- a가 red인 이유 = a와 b가 원래 같은 3-노드였으니까

### 4-노드 = black + red 자식 2개 (2-3-4 트리 / RB의 진짜 대응물)
```
2-3-4:  [a | b | c]        RB:       (b)black
                                    /        \
                                (a)red       (c)red
```
> 2-3 트리에는 4-노드가 없다. red-red가 금지인 이유가 이것.

---

## 2. 균형 조건 대응 — 가장 중요

### black-height = 2-3 트리의 높이

**`red` 노드는 black-height를 세는 데 안 친다.**
→ 이유: red와 그 부모 black은 2-3 트리에서 **하나의 노드**이기 때문.
→ black→red로 내려가는 건 "같은 노드 안에서 이동" (새 레벨 아님).
→ black→black으로 내려갈 때만 2-3 트리에서 진짜 한 층 깊어진다.

| RB 불변식 | 2-3 트리 언어 |
|---|---|
| 모든 경로의 black 개수가 같다 (black-height 일정) | 모든 leaf가 같은 깊이 (perfect balance) |
| "모든 leaf(NIL)는 black" | black-height를 well-defined하게 하는 기준점 |

> NULL 방식 구현(sentinel NIL 없음)에서는 "모든 leaf는 black" 규칙이
> 명시적으로 안 적힐 수 있다 — NULL을 암묵적 black으로 간주해 흡수됨. 개념은 동일.

### perfect balance vs height-balanced
- 2-3 트리: 모든 leaf 깊이 차 **0** (perfect balance) — 더 강함
- AVL: 양쪽 높이 차 **≤ 1** (height-balanced) — 더 약함

---

## 3. 균형을 맞추는 방법의 같은 말 다른 표현

| RB tree | 2-3 tree | 2-3 트리에 보이나? | cascade |
|---|---|---|---|
| **color flip** | split (중앙값 위로) | 보임 (구조 변화) | O |
| **회전 (rotate)** | 같은 3-노드의 표현 조정 | **안 보임** | X (국소적) |
| 회전 + 재색칠 조합 | borrow / merge (삭제) | 보임 (키 이동) | merge는 전파 |

### color flip = split
4-노드 split을 색으로 표현한 것. **포인터는 그대로, 색만 뒤집음 (O(1)).**
```
split 전 (4-노드)          split 후
   (b)black                  (b)red      ← black→red (위로 합류 표시)
   /       \                 /     \
(a)red   (c)red          (a)black (c)black  ← red→black (독립)
```
- b: black→red (부모의 2-3 노드에 합류 = "올라감"을 색으로 표현)
- a, c: red→black (b에서 독립한 별개의 2-노드)
- flip 후 b가 red → 부모도 red면 **red-red 위반** → 위로 전파 (= split의 cascade)
  - 부모가 black → 종료 (2-3-4에서 부모가 4-노드 아니었음 = 키 받을 여유 O)
  - 부모가 red → 또 처리 (2-3-4에서 부모도 4-노드 됨 = 또 split)


### 회전 = 3-노드의 이진 표현 조정 (2-3 트리엔 안 보임)
같은 3-노드 `[a|b]`를 그리는 두 가지 RB 표현:
```
red가 왼쪽:              red가 오른쪽:
   (b)black                (a)black
   /     \                 /     \
(a)red    R              L      (b)red
/   \                           /   \
L    M                         M     R
```
- 둘 다 inorder = `L<a<M<b<R` (완전히 같은 3-노드)
- 이 둘 사이 변환 = **rotate** (b 기준 right-rotate ↔ a 기준 left-rotate)
- 2-3 트리 관점: 둘 다 그냥 `[a|b]` 하나 → **회전은 2-3 구조를 안 바꿈, 표현만 정리**

> - 내 실수: "회전 = borrow(redistribution)" → 틀림.
> - 회전: 2-3 구조 불변, 표현만 조정 (더 원초적)
> - borrow: 2-3 트리에서 키가 실제로 이동 (구조 변화)
> - borrow의 RB 구현이 회전을 **포함**할 뿐, 둘이 같은 건 아니다.

---

## 4. "RB 삭제 회전은 왜 상수 번인가" — 최종 답

(3장 표 참고: 회전은 2-3에 안 보이는 연산이라 국소적 → 전파될 게 없음)

- **회전**은 2-3 트리 구조를 안 바꾸는 "표현 정리"라 **국소적** → 전파될 게 없음.
- 전파되는 것 = **재색칠** (split/merge라는 진짜 구조 변화).
- 그래서:

| | 재색칠 (전파) | 회전 (종료자/국소) |
|---|---|---|
| RB 삽입 | O(log n) | 최대 2번 |
| RB 삭제 | O(log n) | 최대 3번 |

회전이 "종료자"인 이유: 회전 결과 꼭대기에 **black**이 오면 red-red 전파 통로가 막히고
black-height가 보존돼서, 부모 입장에서 변화가 없음 → 위로 검사 불필요.
(AVL에서 "회전 후 높이 불변이면 cascade 멈춤"과 같은 원리. AVL=height, RB=black-height.)

## 한 장 요약

```
2-3(-4) 트리              RB 트리
────────────────────────────────────────────
2-노드 [x]         =      black 단독
3-노드 [a|b]       =      black + red 자식 1개
4-노드 [a|b|c]     =      black + red 자식 2개 (top-down 한정)
높이               =      black-height
모든 leaf 같은 깊이  =      black-height 일정 (perfect balance)
split             =      color flip      (구조 변화, 전파됨)
3-노드 표현 조정     =      rotate          (2-3엔 안 보임, 국소적)
borrow/merge      =      rotate + 재색칠   (키 이동, merge는 전파)

핵심: red = "부모와 한몸" → 여기서 전부 파생.
```
