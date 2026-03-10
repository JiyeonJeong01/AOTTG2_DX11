# Physics_System

# Collider
##  Collider Proxy 빌드 / 충돌 범위 정리

현재 구조에서 실제 충돌 판정은 **원본 ColliderData를 직접 쓰는 것이 아니라**, 매 프레임 `COLLIDER_PROXY_DATA`를 만들어 그 값을 기준으로 진행된다.

즉 흐름은 아래와 같다.

**ColliderData + TransformData → Collider_Proxy_Builder → ColliderProxyData → NarrowPhase 충돌 판정**

---

## 전체 공통 규칙

### 1. Proxy의 공통 기준 중심점
모든 타입은 먼저 `Build_Collider_Proxy()`에서 공통 중심점을 계산한다.

**사용 요소 및 계산식**
- `vCenterWorld = pTr->vPosition + pCol->vOffset`

현재 코드상 `vOffset`은 **Transform 회전을 먹지 않고**, 단순히 월드 위치에 더해진다.

즉 현재 중심점은   **Transform 위치**에 **Collider 로컬 오프셋을 그냥 더한 값**이다.

---

### 2. 현재 Box는 OBB가 아니라 AABB 기준
주석 그대로 현재 구조상 Box 콜라이더는 **무조건 AABB** 로 프록시를 만든다.

그래서 회전이 들어가더라도 충돌 범위 자체는 “회전된 박스”가 아니라, **축 정렬 박스(AABB)** 로 계산됩니다.
추후 OBB로 변경될 가능성이 높다.

---

## Shape별 Proxy 빌드 요약

| 타입 | Proxy 빌드에 사용되는 핵심 요소 | 최종 충돌 범위 계산 방식 | 기타 |
| :--- | :--- | :--- | :--- |
| **BOX** | `vCenterWorld`, `box.vHalfExtentsLocal`, `pTr->vScale` | Local HalfExtent에 Transform Scale을 곱해 `vHalfExtentsWorld`를 만들고, 중심점 ± 반크기로 `aabbWorld` 계산 | 현재는 **항상 AABB** |
| **SPHERE** | `vCenterWorld`, `sphere.fRadiusLocal`, `pTr->vScale` | 반지름에 Transform의 절대 스케일 최댓값을 곱해 `fRadiusWorld` 계산, 중심점 ± 반지름으로 `aabbWorld` 계산 | 비균일 스케일 시 최대축 사용 |
| **PLANE (Infinite)** | `vCenterWorld`, `plane.vNormalLocal`, `pTr->vRotationQuat`, `pCol->vRotationOffset` | 월드 노멀/평면식 계산 후, 아주 큰 값(`INF_EXT`)으로 감싼 가짜 AABB 생성 | 무한 평면이므로 디버그/브로드페이즈용 박스 |
| **PLANE (Finite)** | `vCenterWorld`, `plane.vDimension`, `plane.vNormalLocal`, `pTr->vRotationQuat`, `pCol->vRotationOffset` | 월드 노멀로부터 U/V 축을 만든 뒤, `dimension`의 절반값으로 네 꼭짓점을 구해 그 점들로 `aabbWorld` 계산 | 실제 평면 크기 반영 |

---

## BOX Proxy

### 사용되는 요소
- `outProxy.vCenterWorld`
- `pCol->box.vHalfExtentsLocal`
- `pTr->vScale`

### 계산 과정
Box는 먼저 로컬 반크기(`vHalfExtentsLocal`)를 가져오고,  
여기에 Transform의 스케일을 축별로 곱해서 월드 반크기(`vHalfExtentsWorld`)를 구한다.

**계산식**
- `vHalfExtentsWorld.x = abs(vHalfExtentsLocal.x * pTr->vScale.x)`
- `vHalfExtentsWorld.y = abs(vHalfExtentsLocal.y * pTr->vScale.y)`
- `vHalfExtentsWorld.z = abs(vHalfExtentsLocal.z * pTr->vScale.z)`

그 다음 중심점 기준으로:

- `vMin = vCenterWorld - vHalfExtentsWorld`
- `vMax = vCenterWorld + vHalfExtentsWorld`

를 계산하여 `aabbWorld`를 만든다.

### 정리
즉 Box는 아래 조합으로 최종 world 충돌 범위를 만든다.

**Box Local HalfExtent * Transform Scale + Collider CenterWorld**

### 주의점
현재는 **회전값을 Box 충돌 범위 계산에 반영하지 않는다.**

그래서 Transform이 회전해도 충돌용 박스는 진짜 OBB가 아니라, 그냥 월드축 기준의 **AABB** 이다.

---

## SPHERE Proxy

### 사용되는 요소
- `outProxy.vCenterWorld`
- `pCol->sphere.fRadiusLocal`
- `pTr->vScale`

### 계산 과정
Sphere는 월드 반지름을 구할 때 Transform 스케일의 절대값 중 **최댓값**을 사용한다.

코드상 과정은 아래와 같습니다.

1. `pTr->vScale.x/y/z`의 절대값을 구함
2. 그중 가장 큰 값을 `fMax`로 선택
3. `fRadiusWorld = fRadiusLocal * fMax`

계산식은 다음과 같다.

- `fRadiusWorld = pCol->sphere.fRadiusLocal * max(abs(scaleX), abs(scaleY), abs(scaleZ))`

그 다음 중심점 기준으로:

- `vMin = vCenterWorld - (r, r, r)`
- `vMax = vCenterWorld + (r, r, r)`

를 계산해서 `aabbWorld`를 만든다.

### 정리
즉 Sphere는 아래 조합으로 최종 world 충돌 범위를 만듭니다.

**Sphere Local Radius ** Transform Scale의 최대축 + Collider CenterWorld**

---

## PLANE Proxy 공통

Plane은 Box, Sphere와 달리 “크기”보다 먼저  **월드 기준 평면 방향과 평면식**을 만드는 과정이 중요하다.

### 사용되는 요소
- `outProxy.vCenterWorld`
- `pCol->plane.bInfinite`
- `pCol->plane.vDimension`
- `pCol->plane.vNormalLocal`
- `pTr->vRotationQuat`
- `pCol->vRotationOffset`

### 월드 노멀 계산 과정
Plane은 로컬 노멀(`vNormalLocal`)을 바로 쓰지 않고,  
Transform 회전과 Collider 회전 오프셋을 합쳐서 최종 월드 노멀을 구한다.

과정은 아래와 같다.

1. `vTrRotQ = Transform 회전 쿼터니언`
2. `vColRotQ = Collider의 Euler 회전 오프셋을 쿼터니언으로 변환`
3. `vFinalRotQ = normalize(vTrRotQ * vColRotQ)`
4. `vWorldN = normalize(rotate(vLocalN, vFinalRotQ))`

Plane의 방향은 아래 요소들로 결정된다.

**Plane Local Normal / Transform Rotation / Collider RotationOffset**

---

### Plane Equation(D) 계산

- `fDistanceWorld = -dot(vWorldN, vCenterWorld)`
- `dot(N, P) + D` : signed distance 

---

## PLANE (Infinite)

### 사용되는 요소
- `vCenterWorld`
- `vNormalWorld`
- `fDistanceWorld`
- `bInfinite == true`

### 계산 방식
무한 평면은 실제로 끝이 없기 때문에, 브로드페이즈나 디버그용으로는 진짜 무한 범위를 가질 수 없다.
그래서 현재 구조에서는 매우 큰 상수값 `INF_EXT = 1e6f`를 써서, 중심점 기준 거대한 AABB를 만든다.

**계산식**
- `vMin = center - INF_EXT`
- `vMax = center + INF_EXT`

### 정리
즉 Infinite Plane의 AABB는  
**실제 무한 평면을 대체하기 위한 매우 큰 가짜 범위**이다.

---

## PLANE (Finite)

### 사용되는 요소
- `vCenterWorld`
- `plane.vDimension`
- `vNormalWorld`
- `vAxisUWorld`
- `vAxisVWorld`

### U/V 축 생성 과정
Finite Plane은 단순히 법선만으로는 크기를 표현할 수 없으므로,  법선에 수직인 두 축(U, V)을 만들어 평면의 가로/세로 기준축으로 사용한다.


1. 기준 벡터 `vRef`를 먼저 잡음  
   - 기본은 `(0,1,0)`
   - 노멀과 너무 평행하면 `(1,0,0)`으로 교체
2. `vU = normalize(cross(vRef, vWorldN))`
3. `vV = normalize(cross(vWorldN, vU))`

이 과정을 통해 월드 공간 기준의 평면 축이 만들어진다.

- `vAxisUWorld`
- `vAxisVWorld`

---

### Dimension을 이용한 유한 평면 범위 계산
Finite Plane은 `vDimension`을 실제 평면의 가로/세로 크기로 사용한다.

즉:

- `fHalfW = dimension.x * 0.5f`
- `fHalfH = dimension.y * 0.5f`

를 구한 뒤, 중심점에서 U/V축 방향으로 사각형의 네 꼭짓점을 계산한다.

**꼭짓점 계산**
- `c0 = center + U * halfW + V * halfH`
- `c1 = center + U * halfW - V * halfH`
- `c2 = center - U * halfW + V * halfH`
- `c3 = center - U * halfW - V * halfH`

그 다음 네 점을 모두 순회하면서 min/max를 확장하여  
최종 `aabbWorld`를 만든다.

마지막으로 너무 얇아서 문제가 생기지 않도록 `thickEps`를 더해  
조금 두께 있는 박스로 보정한다.

### 정리

**Local Normal을 회전시켜 월드 Normal 계산 → 그 Normal로부터 U/V 축 생성 → Dimension 절반값으로 네 꼭짓점 계산 → 네 점의 min/max를 구해 AABB 생성**


**Plane은 infinite 하지 않다면 `dimension`을 이용하는데, 이는 `월드 노멀 계산 → U/V 축 생성 → 중심점에서 가로/세로 반길이만큼 네 꼭짓점을 구함 → 그 점들을 감싸는 AABB를 만드는 과정`을 통해 구해진다.**

---

## NarrowPhase에서 실제로 쓰는 값 요약

| 타입 | NarrowPhase에서 주로 참조하는 Proxy 값 |
| :--- | :--- |
| **BOX** | `aabbWorld`, `box.vHalfExtentsWorld`, `vCenterWorld` |
| **SPHERE** | `sphere.fRadiusWorld`, `aabbWorld`, `vCenterWorld` |
| **PLANE** | `plane.vNormalWorld`, `plane.fDistanceWorld`, `plane.vAxisUWorld`, `plane.vAxisVWorld`, `plane.vDimension`, `aabbWorld`, `vCenterWorld` |

---

---

## 핵심 한 줄 요약

### BOX
**Box는 `vHalfExtentsLocal`과 `Transform vScale`을 이용해 `vHalfExtentsWorld`를 만들고, 중심점 ± 반크기로 최종 `aabbWorld`를 구한다.**

### SPHERE
**Sphere는 `fRadiusLocal`에 `Transform vScale`의 절대값 최대축을 곱해 `fRadiusWorld`를 구하고, 중심점 ± 반지름으로 `aabbWorld`를 만든다.**

### PLANE
**Plane은 `local normal + transform rotation + collider rotation offset`으로 월드 노멀을 구하고, infinite 하지 않다면 `dimension`을 이용하는데 이는 `월드 노멀로부터 U/V 축을 만든 뒤 중심점에서 네 꼭짓점을 계산하고, 그 점들을 감싸는 min/max`를 구하는 과정을 통해 만들어진다.**

---

## 최종 정리

| 타입 | 중심점 | 크기 결정 요소 | 회전 반영 | 최종 Proxy 형태 |
| :--- | :--- | :--- | :---: | :--- |
| **BOX** | `Transform Position + Collider Offset` | `HalfExtentLocal × Transform Scale` | X | **AABB** |
| **SPHERE** | `Transform Position + Collider Offset` | `RadiusLocal × max(abs(Scale))` | 불필요 | **World Sphere + AABB** |
| **PLANE (Infinite)** | `Transform Position + Collider Offset` | 무한 대체용 `INF_EXT` | O | **Plane 정보 + Huge AABB** |
| **PLANE (Finite)** | `Transform Position + Collider Offset` | `Dimension`, `U/V Axis`, `Normal` | O | **Plane 정보 + 꼭짓점 기반 AABB** |


---
# Rigidbody

## Rigidbody Body Types 요약

| 타입 | 물리 연산 (중력/힘) | 제어 주체 | 특징 |
| :--- | :---: | :--- | :--- |
| **STATIC** | X | 고정 (Transform) | Rigidbody가 없는 상태. 움직이지 않는 배경, 지형 전용. |
| **KINEMATIC** | X | 스크립트/애니메이션 | 물리 법칙을 무시하지만, 다른 Dynamic 물체를 밀어낼 수 있음. |
| **DYNAMIC** | **O** | 물리 엔진 (PhysX) | 중력, 충돌, 마찰력 등 모든 물리 법칙이 적용되는 일반적인 상태. |

### 핵심 충돌 규칙
* **Static vs Static:** 서로 물리적으로 무시하며 그냥 **뚫고 지나간다.**
* **충돌 발생 조건:** 최소한 어느 한쪽에는 반드시 **Rigidbody**가 있어야 충돌이 발생한다.
* **최적화:** 움직이는 물체에 Rigidbody가 없으면 매 프레임 부하가 발생하므로, 움직인다면 최소한 **Kinematic**이라도 추가해야 한다.

---

## Rigidbody 기초 개념 요약 및 구현

### 1. 물체의 한 점의 속도 : `v_point = v_linear + (ω × r)`
- 개념
    - `v_linear` : 물체 전체가 평행이동하는 **선 속도**
    - `ω × r` : 회전 때문에 그 점에서 추가로 생기는 속도
        -   `ω` : 각 속도 
- 구현
    - `vLinearVel` : 물체의 선속도
    - `vForceAccum` : 이번 프레임에 작용한 선형 힘(Force) 누적값
    - `vTorqueAccum` : 이번 프레임에 작용한 회전 힘(Torque) 누적값
    - `Accum` : 가속도 자체가 아닌 이번 프레임에 들어온 누적할 힘 버퍼
- 흐름
    1. `Accum`에 힘을 누적한다.
    2. `a = F / m` : 가속도를 구한다 
    3. `v = v + a * dt` : 가속도를 이용하여 선 속도를 갱신한다.
    1. 갱신된 `vLinearVel`로 물체의 위치를 이동시킨 뒤, `Accum`은 0으로 초기화한다.


---

## Rigidbody 힘 관련 API 사용 가이드

물리 현상 구현 시 **즉각적 변화**인지 **지속적 축적**인지에 따라 함수를 선택해야 한다.

### 1. API 비교 요약
| 함수명 | 조작 대상 | 반영 시점 | 물리 법칙 | 권장 사용 케이스 |
| :--- | :--- | :--- | :--- | :--- |
| **Add_LinearImpulse** | `vLinearVel` | **즉각 반영** | v += J / m | 점프, 넉백, 폭발 반동, 대시 시작 |
| **Add_Force** | `vForceAccum` | 물리 업데이트 | a = F / m | 엔진 추진력, 바람, 자기력, 제트팩 |
| **Add_Torque** | `vTorqueAccum` | 물리 업데이트 | tau = I alpha | 바퀴/프로펠러 회전력, 지속 회전 함정 |



### 2. 핵심 메커니즘 차이
* **Impulse (충격량)**
   - 가속도 적분 과정 없이 **즉시 속도를 변경**한다. 
   - `Get_KeyDown`처럼 단발성 호출 시 발사되는 느낌을 줄 때 최적이다.
* **Force / Torque (힘/토크)**
    - `Accum` 버퍼에 힘을 쌓은 뒤, `dt`를 곱해 가속도->속도로 변환한다. 
    - 매 프레임 호출하여 물체를 **점진적으로 가속**시킬 때 사용한다.

### 3. 선택 가이드라인
- **지금 당장 속도를 바꿔야 하는가?** -> `Add_LinearImpulse`
- **일정 시간 동안 계속 밀어야 하는가?** -> `Add_Force` / `Add_Torque`

---

## 주의할 점 (엔진 규약)

### plane 콜라이더의 노멀 초기 값은 (0, 1, 0)이다.
