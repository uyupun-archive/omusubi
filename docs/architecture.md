# omusubi Architecture

このドキュメントでは、omusubiフレームワークの設計思想とアーキテクチャの詳細を説明します。

## 設計哲学

### なぜContext Patternを採用したか

omusubiは多様なプラットフォーム間の差異を吸収するため、Context Patternを採用しています。

**背景となる課題:**

- プラットフォームごとにリソース管理の方法が異なる(メモリモデル、ライフサイクル、初期化手順など)
- グローバル変数やシングルトンに依存した設計は移植性が低い
- 実行環境ごとに利用可能なリソースや制約が異なる

**Context Patternの利点:**

- データの配置やライフサイクルを実装側で自由に決定できる
- インターフェースを通じたアクセスにより、実装の詳細を隠蔽
- DIコンテナとして機能し、テスト時のモック差し替えが容易
- 階層構造により関心事を分離

**参考デザインパターン:**

- Abstract Factory Pattern
- Dependency Injection Pattern
- Strategy Pattern

### なぜ継承ではなく合成か

omusubiが`*able`インターフェースの合成を設計の中心に据えているのには、明確な理由があります。

**継承による拡張の問題:**

機能を追加するたびに基底クラスを拡張する設計は、基底の肥大化を招きます。

```cpp
// ❌ 継承で拡張する設計
class Notifier {
    send(message: string)
}
class EmailNotifier extends Notifier {
    // メール送信を追加 → 基底にSMTP設定が必要になる
}
class LoggedEmailNotifier extends EmailNotifier {
    // ログ機能を追加 → 基底にLoggerが必要になる
}
class EncryptedLoggedEmailNotifier extends LoggedEmailNotifier {
    // 暗号化を追加 → 基底にさらに暗号鍵が必要になる
}
// → 組み合わせごとにサブクラスが増え続ける
```

この設計の根本的な問題は、機能の組み合わせが継承ツリーに固定されることです。新しい組み合わせが必要になるたびにサブクラスを追加するしかなくなります。

**合成による拡張:**

代わりに、同一の最小インターフェースを実装する独立したコンポーネントを組み合わせます。

```cpp
// ✅ 同一インターフェースで合成する設計
interface Sendable {
    send(message: string)
}

// 送信手段(実体)
class EmailSender implements Sendable { /* SMTPホスト、ポート、認証情報を保持 */ }
class SlackSender implements Sendable { /* Webhook URLを保持 */ }

// 加工層(Sendableを受け取り、自身もSendableとして振る舞う)
class Logger    implements Sendable { /* 内部にSendableを保持し、送信前にログを記録 */ }
class Encrypter implements Sendable { /* 内部にSendableを保持し、送信前に暗号化 */ }
```

```cpp
// メール送信 + ログ + 暗号化
// 実行順: Encrypter → Logger → EmailSender(外側から内側へ実行)
notifier: Sendable = Encrypter(Logger(EmailSender(smtp_host, smtp_port)))

// Slackに切り替え + 順序変更も自由
notifier: Sendable = Logger(Encrypter(SlackSender(webhook_url)))

// 送信側は何も変わらない
notifier.send(message)
```

**この設計が成立する条件:**

- **入力の型 = 出力の型**: 各コンポーネントが同じインターフェースを受け取り、同じインターフェースを公開する
- **各コンポーネントは変換の1ステップだけを担当する**: 単一責任
- **内部の複雑さは外部から見えなくて良い**: インターフェースの表面積を最小にすることで、内部の自由度を最大にする

omusubiの`*able`パターンはこの原則の体現です。`Readable`, `Writable`, `Connectable`といった最小インターフェースが、継承ツリーではなく合成によって機能を構築する基盤となっています。

**参考デザインパターン:**

- Decorator Pattern(同一インターフェースによる機能の積み重ね)

**パラメータ化による汎用化:**

同一の処理パターンでパラメータだけが異なるケースでは、サブクラスを量産するのではなく、汎用的な処理エンジンにパラメータを渡す構成を取ります。

```cpp
// ✅ 汎用エンジン + パラメータ
sensor_config = [
    { type: "temperature", address: 0x48, interval_ms: 1000 },
    { type: "pressure",    address: 0x77, interval_ms: 500 },
]
engine = SensorEngine(sensor_config)

// ❌ パラメータごとにサブクラス
class TemperatureSensor1000ms extends Sensor { ... }
class PressureSensor500ms extends Sensor { ... }
```

### プラットフォーム固有制約への対応方針

omusubiでは、プラットフォーム固有の制約への対応は各実装に委ねます。フレームワーク自体は制約を強制せず、各環境のベストプラクティスに従えるよう設計されています。

**プラットフォーム実装が考慮すべき事項:**

| 観点 | 説明 | 例 |
| --- | ---- | --- |
| メモリモデル | リソースの確保・解放戦略 | スタック割り当て、ヒープ、プール、GC管理 |
| エラーハンドリング | エラー通知の手段 | 例外、Result型、エラーコード |
| 並行性 | スレッドモデルと同期 | シングルスレッド、async/await、RTOS |
| 型情報 | 実行時型情報の利用可否 | RTTI、リフレクション |
| コンパイル時計算 | 静的解析・最適化の活用 | constexpr、ジェネリクス、マクロ |

> **リファレンス実装(C++17組み込み)の制約例:**
> ゼロオーバーヘッド抽象化を実現するため、ヒープ確保禁止、例外禁止、RTTI禁止、全オブジェクトのスタック割り当て、固定サイズコンテナの使用を採用しています。
> これらはC++17組み込み環境での最適な選択であり、他のプラットフォームでは異なるアプローチを取り得ます。

### インターフェース分離原則(ISP)の適用

**単一責任のインターフェース設計:**

各`*able`インターフェースは単一の責任を持ちます:

```cpp
// ✅ 良い例: 単一責任
interface Readable {
    read(buffer: byte[], length: size) -> size
    available() -> bool
    // 読み取りに関連するメソッドのみ
}

// ❌ 悪い例: 複数責任の混在
interface NetworkDevice {
    connect()      // 接続責任
    scan()         // スキャン責任
    read()         // 読み取り責任
}
```

**インターフェースの組み合わせ:**

複数の機能を持つデバイスは、複数のインターフェースを実装します:

```cpp
class SerialAccessor implements TextReadable, TextWritable, Connectable {
    // Serial通信には読み書き＋接続管理が必要
}
```

**抽象化のポリシー:**

- **操作(動詞)は抽象化可能** - `connect()`, `read()`, `write()`等のアクションはインターフェース化
- **構造(データ)は抽象化しない** - 値型(文字列、ベクトル等)は具象型を直接使用
- 構造を抽象化すると必ず抽象化漏れが発生するため、具象型を使用することで明確化

### リソースの分類

```text
外部リソース
├─ 入力(データソース) → Measurable / Readable 系
│  ├─ スカラー値 → get_value()
│  └─ 複合値   → get_values()
│
└─ 出力(データシンク) → Controllable / Writable 系
   ├─ ON/OFF   → Switchable
   └─ 連続制御 → PositionControllable, SpeedControllable
```

**入出力の分離:**

```cpp
// ✅ 分離する
class PumpAccessor         implements Switchable { }   // 出力
class FlowSensorAccessor   implements Measurable { }   // 入力

// ❌ 混在させない
class SmartPump implements Switchable, Measurable { }
```

この分離はSRPに基づくものであり、入力と出力は本質的に異なる関心事です。物理的に同一のデバイスであっても、論理的な責務で分離します。

## アーキテクチャ原則

設計哲学(Why)を実際のコード構造に落とし込む際の指針(What)です。

### 依存方向の制約 - ピラミッド構造

オブジェクト間の依存関係は常にピラミッド型(ツリー構造)になるよう設計します。同じ階層のオブジェクト同士が直接やり取りすることを禁止し、必ず上位を介します。

```text
// ✅ ピラミッド構造: 上下の関係のみ
//
//            Manager
//           ↗      ↖
//     ComponentA   ComponentB
//
// AがBの情報を必要とする場合:
//   A → Manager に問い合わせ → Manager が応答
//   (Managerは抽象化した情報のみ返す)

manager.is_area_available(position)   // ✅ 抽象化された問い合わせ

// ❌ 横の関係: 同レベルのオブジェクト間で直接アクセス
//
//     ComponentA ←→ ComponentB
//
component_b.get_position()            // ❌ 隣のオブジェクトの内部状態に依存
```

**この原則が保証すること:**

- 依存グラフが循環しない(DAGが保たれる)
- コンポーネントの追加・削除が局所的な変更で済む
- 各コンポーネントは自分の上位だけを知ればよい

omusubiの`SystemContext → Provider → Accessor`はこのピラミッド構造そのものです。同一レイヤーのオブジェクト同士が直接参照することはなく、上位を介してのみ間接的にやり取りします。

### 上位層の振る舞い - 指示と実行の分離

上位のマネージャ(SystemContextやProvider)は「何をすべきか」を指示しますが、「どうやるか」には介入しません。具体的な実行は各コンポーネントの自律的な判断に委ねます。

omusubiでは、SystemContextがこの上位管理者の役割を担います。

```cpp
// ✅ 指示のみ: 上位は制約を伝え、実行は下位に任せる
component.set_boundary(limit)         // 「ここまでが範囲」という制約
component.notify_state_change(state)  // 「状態が変わった」という事実

// ❌ 介入: 上位が下位の内部処理を代行する
component.position = calculate_new_position(component)  // 上位が位置を計算して直接代入
component.internal_buffer.resize(new_size)              // 上位が内部バッファを直接操作
```

**公平性の原則:** 上位層が同レベルの複数コンポーネントを管理する場合、特定のコンポーネントに他のコンポーネントの内部情報を伝えてはいけません。伝える情報は常に抽象化されたものに限定します。

```cpp
// ❌ 贔屓: Aの内部状態をBに教える
component_b.update(component_a.get_internal_state())

// ✅ 抽象化: 共通のインターフェースで同等に扱う
for component in components:
    component.update(shared_info)  // 全員が同じ情報を受け取る
```

### ライフサイクル管理 - 生成と利用の分離

オブジェクトの生成・破棄はその利用者ではなく、ファクトリクラスや上位のマネージャが担います。オブジェクト自身は与えられた環境の中で動作することだけに集中します。

```cpp
// ✅ 生成と利用の分離
//
// ファクトリクラスが生成:
sensor: Measurable = SensorFactory.create(config)
manager.register(sensor)

// 利用側は生成方法を知らない:
class DataLogger {
    sensor: Measurable    // インターフェースで保持

    log() {
        value = this.sensor.get_value()  // 利用のみ
    }
}

// ❌ 利用者が生成も行う
class DataLogger {
    sensor = I2CTemperatureSensor(0x48)  // 具象型と初期化パラメータに依存
}
```

**確保と解放はワンセット:** リソースを確保したコードが解放の責務も持ちます。利用側にリソースの寿命管理を押し付けないことで、リソースリークを構造的に防ぎます。

### イベントモデル - Pull型を基本とする

コンポーネント間のイベント連携は、各コンポーネントが自律的にイベントキューを確認するPull型を基本とします。

```cpp
// Pull型: 各コンポーネントが必要に応じてチェックする
class Component {
    update(context) {
        if context.event_queue.has(MY_EVENT_TYPE):
            event = context.event_queue.get(MY_EVENT_TYPE)
            this.handle(event)
    }
}
```

**イベントキューの配置:** イベントキューはSystemContextまたはProviderの一部として保持します。これにより、イベントの発行・購読が上位を経由する形となり、ピラミッド構造の依存方向制約と整合します。

### 入力の抽象化 - ハードウェアと意図の分離

ハードウェア入力を直接使うのではなく、「アプリケーションにとって理想的な入力インターフェース」を先に定義し、ハードウェアからの変換は別のレイヤーで行います。

**変換の3段階:**

1. **Raw Input**: ハードウェアからの生データ(ADC値、GPIOレベル、タッチ座標等)
2. **Calibrated Input**: キャリブレーション済みの正規化データ
3. **Intent**: アプリケーションにとって意味のある操作(移動、選択、キャンセル等)

アプリケーションコードはIntentのみに依存します。

```cpp
// Intentレベルのインターフェース(アプリケーション側が定義)
interface NavigationInput {
    get_move() -> Vector2D        // 移動方向と量
    is_select() -> bool           // 決定操作
    is_cancel() -> bool           // 取り消し操作
}

// ハードウェアごとにIntent変換を実装
class GamepadAdapter  implements NavigationInput { ... }   // スティック+ボタン
class TouchAdapter    implements NavigationInput { ... }   // スワイプ+タップ
class KeyboardAdapter implements NavigationInput { ... }   // WASD+Enter+Esc

// アプリケーション側はどの入力デバイスでも同じコード
navigation: NavigationInput = GamepadAdapter(gamepad_id)
move: Vector2D = navigation.get_move()
```

### リソース管理の独立性 - リソースと利用者の分離

リソース(メモリ、ペリフェラル、外部接続等)の確保・解放は利用者から独立したマネージャが自律的に行います。利用者はリソースの有無や状態遷移を意識しません。

```cpp
// リソースマネージャが自律的に管理
class ResourceManager {
    // リソースとアプリケーション状態の対応テーブル
    mapping = {
        "state_a": [resource_1, resource_2],
        "state_b": [resource_2, resource_3],
    }

    on_state_change(next_state: string) {
        needed = this.mapping[next_state]
        this.release_unused(needed)
        this.acquire_needed(needed)
    }
}

// 利用側はリソースの確保状態を知らない
class ModuleA {
    update(context) {
        data = context.get_resource("resource_1")  // 使うだけ
    }
}
```

### 表示とロジックの分離 - 出力は実装の詳細

状態を管理するロジックと、それを外部に表現する処理を分離します。出力側に判断ロジックを持たせません。

```cpp
// Application Code: 状態の判断ロジック
class TemperatureMonitor {
    sensor: Measurable
    threshold: float

    evaluate() -> DisplayState {
        temp: float = this.sensor.get_value()
        return DisplayState {
            value: temp,
            is_alert: temp > this.threshold,
        }
    }
}

// Application Code: 出力の表現(状態に応じて表示するだけ)
class TemperatureRenderer {
    display: Displayable

    render(state: DisplayState) {
        if state.is_alert:
            this.display.show("ALERT: " + state.value)
        else:
            this.display.show("Temp: " + state.value)
    }
}
```

omusubiのデバイス操作(Writable, Displayable等)は出力の手段であり、その出力内容を決定するロジックはApplication Code側に配置します。

### フレームワークのフェーズ構造

フレームワークの基本ライフサイクルは`initialize`と`destroy`の2フェーズです。

> **omusubiが定義するのはフェーズのインターフェースです。各フェーズをいつ・どのように呼び出すかは実装者が決定します。**

```cpp
// 基本ライフサイクル
interface Lifecycle {
    initialize()    // 初期状態設定
    destroy()       // 終了処理
}
```

中間フェーズ(状態更新、出力等)は、アプリケーションの特性に応じてインターフェースを追加します。

```cpp
// 例: ループ型アプリケーション向けの拡張
interface Updatable {
    update()        // 状態更新
}

interface Renderable {
    render()        // 出力
}
```

## レイヤー構造

omusubiは5つのレイヤーで構成されています。上位レイヤーは下位レイヤーに依存しますが、逆方向の依存は禁止です。

```text
│ # | Application Code (ユーザーコード) │
| --- | --- |
│ 1 | Interface Layer - 契約の定義 │
│ 2 | Provider Layer  - 分類と提供 │
│ 3 | Accessor Layer  - リソースアクセス定義 │
│ 4 | Component Layer - サブコンポーネント(任意) │
│ 5 | Platform Layer  - プラットフォーム固有実装 │
```

### 命名規則

オブジェクトの命名はそのレイヤーでの役割を反映します。

| レイヤーの役割 | サフィックス | 責務 | 例 |
| --- | --- | --- | --- |
| 管理・所有 | `*Context` | 下位の生成・破棄・ライフサイクル管理 | `SystemContext` |
| 分類・提供 | `*Provider` | 同種の能力を持つものをグルーピングし提供 | `ConnectableProvider`, `SensorProvider` |
| 操作・接続 | `*Accessor` | 特定のリソースへのアクセス手段 | `SerialAccessor`, `WiFiAccessor` |
| ドメイン概念 | ドメイン標準用語 | リソース内部のドメイン固有オブジェクト | `BLECharacteristic`, `HTTPRequest` |

**Getter命名規則:**

```cpp
// SystemContext → Provider
get_connectable_provider() const -> ConnectableProvider
get_sensor_provider() const -> SensorProvider

// Provider → Accessor
get_serial_accessor() const -> SerialAccessor
get_serial_accessor(port: uint8) const -> SerialAccessor
get_serial_accessor<Port>() const -> SerialAccessor

// 複数インスタンスの数量取得
get_serial_count() const -> uint8

// ❌ 禁止パターン
serial_accessor()         // get_プレフィックスなし
get_serial()              // _accessor サフィックスなし
get_serial_1_accessor()   // 同一型の複数getter禁止(パラメータ化すべき)
```

### 1. Interface Layer

**責務:** 単一責任の契約定義

**`*able`インターフェース:**

- `ByteReadable` / `TextReadable` - データ読み取り
- `ByteWritable` / `TextWritable` - データ書き込み
- `Connectable` - 接続管理
- `Scannable` - デバイススキャン
- `Pressable` - ボタン状態
- `Measurable` / `Measurable3D` - センサー測定
- `Displayable` - ディスプレイ出力

**設計ルール:**

- 各インターフェースは単一責任
- 関連するメソッドは同一インターフェースに含めてOK(例: `start_scan()` + `stop_scan()`)
- 純粋な契約のみ(実装を含まない)
- 値のセマンティクス: コピー・ムーブは原則禁止(アイデンティティを持つ)

### 2. Provider Layer

**責務:** 同種のAccessorの分類と提供

**Provider一覧:**

- `ConnectableProvider` - すべての接続可能デバイス
- `ScannableProvider` - すべてのスキャン可能デバイス
- `SensorProvider` - すべてのセンサーデバイス
- `InputProvider` - すべての入力デバイス
- `OutputProvider` - すべての出力デバイス
- `SystemInfoProvider` - システム情報(読み取り専用)
- `PowerProvider` - 電源管理

**Providerの役割:**

- Accessorの生成と所有権管理
- テスト時のモック差し替え
- プラットフォーム切り替えの抽象化

### 3. Accessor Layer

**責務:** 特定のリソースへのアクセス手段の定義

**Accessor一覧:**

- `SerialAccessor` - シリアル通信(`TextReadable + TextWritable + Connectable`)
- `BluetoothAccessor` - Bluetooth通信(`TextReadable + TextWritable + Connectable + Scannable`)
- `WiFiAccessor` - WiFi接続(`Connectable + Scannable`)
- `BLEAccessor` - BLE通信(`Connectable + Scannable`)

**複数インスタンスを持つAccessorの扱い:**

```cpp
// ✅ パラメータで識別
get_serial_accessor(port: uint8) const -> SerialAccessor
get_serial_count() const -> uint8

// ❌ 禁止: 同一型の複数getter
get_serial_1_accessor() -> SerialAccessor
get_serial_2_accessor() -> SerialAccessor
```

### 4. Component Layer(任意、複雑なAccessor用)

**責務:** Accessor内部のサブコンポーネント表現

**使用する場合:**

- Accessorが動的に作成可能なサブコンポーネントを持つ
- サブコンポーネントが独自の状態とメソッドを持つ
- サブコンポーネントが独立したドメイン概念を表す

#### 例: BLE階層構造

```cpp
// 基本(3層): SystemContext → Provider → Accessor
ble = ctx.get_connectable_provider().get_ble_accessor()

// 階層的(4層+): → Component(Service, Characteristic)
service = ble.add_service(service_uuid)
ch = service.add_characteristic(char_uuid, properties)
ch.write("data")
```

**命名規則(重要):**

```cpp
// ✅ ドメイン標準用語を使用
BLECharacteristic, BLEService    // BLE仕様の用語
HTTPRequest, HTTPResponse        // Web標準の用語
File, Directory                  // OS標準の用語

// ❌ *Accessor / *Provider サフィックスは禁止(Componentは概念が異なる)
BLECharacteristicAccessor        // 間違い
BLEServiceProvider               // 間違い
```

### 5. Platform Layer

**責務:** プラットフォーム固有の実装

**実装時の指針:**

- Interface/Provider/Accessor Layerで定義された契約をプラットフォーム固有の手段で実装する
- プラットフォーム固有の型や依存はヘッダーに露出させず、実装ファイルに隠蔽する
- Platform Layer以外のコードからプラットフォーム固有APIを直接呼び出さない
**プラットフォーム固有実装の隠蔽原則:**

```cpp
// ❌ ヘッダーに実装詳細を露出
class ConcreteBluetoothAccessor {
    impl_: void*              // 実装ポインタが見える
    InternalState state_      // プラットフォーム固有型が見える
}

// ✅ 実装ファイルに完全隠蔽
// (Pimpl, anonymous namespace, モジュール内部など手段は問わない)
// ヘッダーからはインターフェースのみ公開
```

## SystemContextアクセスパターン

### Free Function Pattern(必須)

```cpp
// グローバル関数(唯一のアクセス方法)
get_system_context() -> SystemContext&

// ✅ 使用例
ctx = get_system_context()

// ❌ 禁止(存在しない)
ctx = SystemContext.get_instance()
```

プラットフォームごとに`get_system_context()`の実装を提供します。ビルドシステムでリンクするファイルを切り替えるだけで、ユーザーコードの変更なくプラットフォームを切り替えられます。

## Method Chain Design

### 基本パターン(3層)

```cpp
SystemContext → Provider → Accessor → method()
```

**使用例:**

```cpp
// シリアル通信
ctx.get_connectable_provider().get_serial_accessor(0).write("Hello")

// WiFi接続
ctx.get_connectable_provider().get_wifi_accessor().connect_to(ssid, password)

// センサー読み取り
ctx.get_sensor_provider().get_accelerometer_accessor().get_values()
```

### 階層パターン(4層+)

```text
SystemContext → Provider → Accessor → Component → method()
```

**使用例:**

```cpp
// BLE: Service と Characteristic
ble = ctx.get_connectable_provider().get_ble_accessor()
service = ble.add_service(uuid)
ch = service.add_characteristic(uuid, props)
ch.write("Hello")
```

## 参考資料

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- Design Patterns: Abstract Factory, Dependency Injection, Strategy, Decorator
- Interface Segregation Principle (SOLID)
- Dependency Inversion Principle (SOLID)
- Android Application Context design

-----

**Version:** 1.0.0
**Last Updated:** 2026-02-15
