// Qt JavaScript 模块（QJSEngine）综合示例
//
// 本示例演示：
//   1) 用 QJSEngine 加载并执行 JS 文件
//   2) 把 C++ 的 QObject 暴露成 JS 全局对象
//   3) 在 JS 里调用 C++ 的槽函数和属性
//   4) 在 C++ 里调用 JS 函数并读取返回值
//   5) 异常 / 错误处理
//   6) 通过 QJSEngine 注册 C++ 可被 JS new 出来的类型
//
// 编译需要：Qt 5.12+ 或 Qt 6（Qml 模块提供 QJSEngine）

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>

#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>
#include <QtQml/QJSValueIterator>
#include <QtQml/QQmlEngine>

// ---------------------------------------------------------------------------
// 一个普通 QObject：用来从 JS 端访问其属性 / 槽 / 信号
// ---------------------------------------------------------------------------
class AppApi : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(int counter READ counter WRITE setCounter NOTIFY counterChanged)

public:
    explicit AppApi(QObject *parent = nullptr) : QObject(parent) {}

    QString appName() const { return QStringLiteral("Qt JS Example"); }

    int counter() const { return m_counter; }
    void setCounter(int v)
    {
        if (m_counter == v) return;
        m_counter = v;
        emit counterChanged(v);
    }

public slots:
    // JS 里可以直接调用：app.log("hello")
    void log(const QString &msg) { qInfo().noquote() << "[JS->C++]" << msg; }

    // 返回值能被 JS 接收
    int add(int a, int b) const { return a + b; }

    // 把任意 JS 对象 / 数组 反序列化到 C++（用 QVariant）
    QVariant echo(const QVariant &v) const { return v; }

signals:
    void counterChanged(int value);
    void tick(int n);
};

// ---------------------------------------------------------------------------
// 一个可被 JS 用 new 创建的类型：演示 QJSEngine::newQObject + 构造函数
// ---------------------------------------------------------------------------
class Vector2 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double x MEMBER x)
    Q_PROPERTY(double y MEMBER y)

public:
    Q_INVOKABLE Vector2(double x_ = 0.0, double y_ = 0.0, QObject *parent = nullptr)
        : QObject(parent), x(x_), y(y_) {}

    Q_INVOKABLE double length() const { return std::sqrt(x * x + y * y); }
    Q_INVOKABLE QString toString() const
    {
        return QStringLiteral("Vector2(%1, %2)").arg(x).arg(y);
    }

    double x = 0.0;
    double y = 0.0;
};

// ---------------------------------------------------------------------------
// 工具函数：把脚本文件读入字符串
// ---------------------------------------------------------------------------
static QString readScript(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open script:" << path << f.errorString();
        return {};
    }
    return QTextStream(&f).readAll();
}

// ---------------------------------------------------------------------------
// 通用错误处理：检查 JS 求值结果是否为异常
// ---------------------------------------------------------------------------
static bool checkError(const QJSValue &result, const QString &where)
{
    if (!result.isError()) return false;
    qCritical().noquote()
        << "JS Error in" << where
        << "\n  message :" << result.toString()
        << "\n  file    :" << result.property("fileName").toString()
        << "\n  line    :" << result.property("lineNumber").toInt()
        << "\n  stack   :\n" << result.property("stack").toString();
    return true;
}

// ---------------------------------------------------------------------------
// 演示 1：直接执行字符串脚本
// ---------------------------------------------------------------------------
static void demoEvaluateString(QJSEngine &engine)
{
    qInfo() << "\n=== Demo 1: evaluate(string) ===";
    const QJSValue r = engine.evaluate(
        "function fib(n){ return n<2 ? n : fib(n-1)+fib(n-2); } fib(12);");
    if (!checkError(r, "demoEvaluateString"))
        qInfo() << "fib(12) =" << r.toInt();
}

// ---------------------------------------------------------------------------
// 演示 2：从文件加载脚本，并调用其中定义的函数
// ---------------------------------------------------------------------------
static void demoEvaluateFile(QJSEngine &engine, const QString &path)
{
    qInfo() << "\n=== Demo 2: evaluate(file) ===" << path;
    const QString src = readScript(path);
    if (src.isEmpty()) return;

    const QJSValue r = engine.evaluate(src, path);
    if (checkError(r, path)) return;

    // 调用脚本里定义的 greet(name)
    QJSValue greet = engine.globalObject().property("greet");
    if (greet.isCallable()) {
        QJSValue ret = greet.call({ QJSValue("Qt") });
        if (!checkError(ret, "greet()"))
            qInfo().noquote() << "greet(\"Qt\") =>" << ret.toString();
    }
}

// ---------------------------------------------------------------------------
// 演示 3：把 C++ 对象注入到 JS 全局
// ---------------------------------------------------------------------------
static void demoInjectQObject(QJSEngine &engine, AppApi *api)
{
    qInfo() << "\n=== Demo 3: expose QObject to JS ===";
    QJSValue jsApi = engine.newQObject(api);
    QQmlEngine::setObjectOwnership(api, QQmlEngine::CppOwnership);
    engine.globalObject().setProperty("app", jsApi);

    const QJSValue r = engine.evaluate(R"JS(
        app.log("Hello from JavaScript, app name = " + app.appName);
        app.counter = 41;
        app.counter = app.counter + 1;          // -> 42
        var sum = app.add(3, 4);                // C++ 槽
        var echoed = app.echo({a:1, b:[2,3]});  // QVariant 互转
        ({ counter: app.counter, sum: sum, echoed: echoed });
    )JS");
    if (checkError(r, "demoInjectQObject")) return;

    qInfo().noquote() << "result =" << r.toVariant();
    qInfo() << "counter (C++ side) =" << api->counter();
}

// ---------------------------------------------------------------------------
// 演示 4：在 C++ 中调用 JS 函数，并把 C++ 信号转到 JS 回调
// ---------------------------------------------------------------------------
static void demoSignalToJsCallback(QJSEngine &engine, AppApi *api)
{
    qInfo() << "\n=== Demo 4: C++ signal -> JS callback ===";

    // 在 JS 里定义一个回调
    QJSValue setup = engine.evaluate(R"JS(
        (function() {
            return function onTick(n) {
                app.log("tick received in JS, n = " + n);
            };
        })()
    )JS");
    if (checkError(setup, "demoSignalToJsCallback/setup")) return;

    // 把信号连到 JS 函数
    // QJSEngine 支持把 callable 直接 connect 到 QObject 的信号：
    QJSValue connectFn = engine.globalObject()
                            .property("app")
                            .property("tick")
                            .property("connect");
    if (connectFn.isCallable()) {
        QJSValue conRet = connectFn.callWithInstance(
            engine.globalObject().property("app").property("tick"),
            { setup });
        checkError(conRet, "tick.connect");
    }

    // 触发几次信号
    for (int i = 1; i <= 3; ++i) emit api->tick(i);
}

// ---------------------------------------------------------------------------
// 演示 5：注册可被 JS new 的 C++ 类型
// ---------------------------------------------------------------------------
static void demoNewableType(QJSEngine &engine)
{
    qInfo() << "\n=== Demo 5: JS-newable C++ type (Vector2) ===";

    // 注册 Vector2 元类型，让 JS 能 new Vector2(x,y)
    QJSValue ctor = engine.newQMetaObject(&Vector2::staticMetaObject);
    engine.globalObject().setProperty("Vector2", ctor);

    const QJSValue r = engine.evaluate(R"JS(
        var v = new Vector2(3, 4);
        ({ str: v.toString(), len: v.length() });
    )JS");
    if (!checkError(r, "demoNewableType"))
        qInfo().noquote() << "result =" << r.toVariant();
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QJSEngine engine;
    engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

    AppApi api;

    demoEvaluateString(engine);

    // 默认从可执行文件目录的 ./scripts 加载
    const QString scriptsDir = QCoreApplication::applicationDirPath() + "/scripts";
    demoEvaluateFile(engine, scriptsDir + "/hello.js");

    demoInjectQObject(engine, &api);
    demoSignalToJsCallback(engine, &api);
    demoNewableType(engine);

    // 再加载一段使用 app 对象的复杂脚本
    qInfo() << "\n=== Demo 6: complex script using app ===";
    const QString src = readScript(scriptsDir + "/app_logic.js");
    if (!src.isEmpty()) {
        QJSValue r = engine.evaluate(src, "app_logic.js");
        checkError(r, "app_logic.js");
    }

    // 让事件循环跑一下后退出（用于演示信号 / 定时器）
    QTimer::singleShot(0, &app, &QCoreApplication::quit);
    return app.exec();
}

#include "main.moc"
