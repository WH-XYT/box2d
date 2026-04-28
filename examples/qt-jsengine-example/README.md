# Qt JavaScript 模块（QJSEngine）脚本示例

本示例展示如何在 C++ 程序里嵌入 [Qt JavaScript 引擎](https://doc.qt.io/qt-6/qjsengine.html)
（`QJSEngine`，位于 `Qt Qml` 模块），把 JavaScript 当作脚本语言用，并实现 C++ 与 JS 双向调用。

> 适用 Qt 5.12+ 与 Qt 6（在 Qt 6 中 `QJSEngine` 仍由 `Qt6::Qml` 提供）。

## 功能演示

`main.cpp` 中按顺序演示了 6 个常见用法：

| 编号 | 主题                                    | 关键 API                                              |
| ---- | --------------------------------------- | ----------------------------------------------------- |
| 1    | 直接执行字符串脚本                      | `QJSEngine::evaluate(QString)`                        |
| 2    | 从 `.js` 文件加载并调用其中函数         | `QJSValue::call`                                      |
| 3    | 把 C++ `QObject` 暴露给 JS（属性 / 槽） | `QJSEngine::newQObject` + `globalObject().setProperty`|
| 4    | 把 C++ 信号连到 JS 回调                 | JS 端 `signal.connect(handler)`                       |
| 5    | 在 JS 里 `new` C++ 类型                 | `QJSEngine::newQMetaObject`                           |
| 6    | 复杂业务脚本（`scripts/app_logic.js`）  | 综合使用以上能力                                      |

并且在 `checkError()` 里集中处理 JS 异常（含 fileName / lineNumber / stack）。

## 目录结构

```
qt-jsengine-example/
├── CMakeLists.txt              # CMake 构建（推荐）
├── qt_jsengine_example.pro     # qmake 构建（可选）
├── main.cpp                    # 宿主程序
├── scripts/
│   ├── hello.js                # 最简单脚本
│   └── app_logic.js            # 使用注入 C++ 对象的脚本
└── README.md
```

## 构建与运行

### 用 CMake

```bash
cd examples/qt-jsengine-example
cmake -S . -B build -DCMAKE_PREFIX_PATH=<你的 Qt 安装前缀>
cmake --build build
./build/qt_jsengine_example
```

`CMAKE_PREFIX_PATH` 例如：
- Linux: `/opt/Qt/6.7.0/gcc_64`
- macOS: `~/Qt/6.7.0/macos`
- Windows (MSVC): `C:/Qt/6.7.0/msvc2019_64`

### 用 qmake

```bash
cd examples/qt-jsengine-example
qmake
make            # Windows 上根据工具链改成 nmake / mingw32-make
./qt_jsengine_example
```

## 期望输出（节选）

```
=== Demo 1: evaluate(string) ===
fib(12) = 144

=== Demo 2: evaluate(file) === ".../scripts/hello.js"
hello.js loaded, PI = 3.1416
greet("Qt") => Hello, Qt! (from hello.js)

=== Demo 3: expose QObject to JS ===
[JS->C++] Hello from JavaScript, app name = Qt JS Example
result = QMap(("counter", 42)("echoed", QMap(...))("sum", 7))
counter (C++ side) = 42

=== Demo 4: C++ signal -> JS callback ===
[JS->C++] tick received in JS, n = 1
[JS->C++] tick received in JS, n = 2
[JS->C++] tick received in JS, n = 3

=== Demo 5: JS-newable C++ type (Vector2) ===
result = QMap(("len", 5)("str", "Vector2(3, 4)"))

=== Demo 6: complex script using app ===
[JS->C++] app_logic.js running, appName = Qt JS Example
[JS->C++] sum(1..10) via C++ add() = 55
...
```

## 关键点说明

### 1. 暴露 QObject

只有 `Q_PROPERTY` 标记的属性、`public slots:` 里的函数、以及 `Q_INVOKABLE` 标记的方法
才会被 `QJSEngine` 暴露给 JS。信号默认带有 `connect` / `disconnect` 方法。

### 2. 对象所有权

`engine.newQObject(obj)` 默认会把对象的所有权交给 JS 引擎（GC 时会被删除）。如果 C++ 端
还要继续使用，需要：

```cpp
QQmlEngine::setObjectOwnership(api, QQmlEngine::CppOwnership);
```

### 3. 异常处理

`QJSValue::isError()` 为真时即代表脚本抛了异常，可以读取 `message / fileName /
lineNumber / stack` 等属性。建议每次 `evaluate` / `call` 后都用一个统一的
`checkError()` 包一下。

### 4. ConsoleExtension

```cpp
engine.installExtensions(QJSEngine::ConsoleExtension);
```

之后 JS 里就能用 `console.log / warn / error` 输出到 Qt 日志系统。

### 5. 让 JS 能 `new` C++ 类型

```cpp
QJSValue ctor = engine.newQMetaObject(&Vector2::staticMetaObject);
engine.globalObject().setProperty("Vector2", ctor);
```

要求构造函数是 `Q_INVOKABLE`，所有要被 JS 看到的方法 / 属性同样要 `Q_INVOKABLE`
或 `Q_PROPERTY`。

## 参考

- [QJSEngine](https://doc.qt.io/qt-6/qjsengine.html)
- [QJSValue](https://doc.qt.io/qt-6/qjsvalue.html)
- [Qt QML JavaScript Host Environment](https://doc.qt.io/qt-6/qtqml-javascript-hostenvironment.html)
