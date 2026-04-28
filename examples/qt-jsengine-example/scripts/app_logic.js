// app_logic.js —— 演示在 JS 中使用注入的 C++ 对象 `app`
// 期望全局存在：
//   app   : C++ 端的 AppApi（QObject）
//   Vector2 : C++ 端注册的可 new 类型

(function () {
    "use strict";

    app.log("app_logic.js running, appName = " + app.appName);

    // 1) 用 C++ 的 add() 做点计算
    var s = 0;
    for (var i = 1; i <= 10; ++i) s = app.add(s, i);
    app.log("sum(1..10) via C++ add() = " + s);

    // 2) 操作属性（带 NOTIFY 信号）
    app.counter = 100;
    app.log("counter set to " + app.counter);

    // 3) 用 Vector2 做点几何
    var vs = [
        new Vector2(1, 0),
        new Vector2(0, 1),
        new Vector2(3, 4),
        new Vector2(-5, 12),
    ];
    vs.forEach(function (v) {
        app.log(v.toString() + " length = " + v.length().toFixed(3));
    });

    // 4) 异常会被 C++ 端 checkError 捕获
    try {
        JSON.parse("not-json");
    } catch (e) {
        app.log("caught in JS: " + e.message);
    }

    // 5) 把对象 echo 一圈，验证 QVariant 互转
    var echoed = app.echo({ list: [1, 2, 3], name: "demo" });
    app.log("echoed.name = " + echoed.name + ", echoed.list.length = " + echoed.list.length);
})();
