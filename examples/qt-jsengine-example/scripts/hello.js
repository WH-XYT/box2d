// hello.js —— 由 QJSEngine 加载的最简单脚本

function greet(name) {
    return "Hello, " + name + "! (from hello.js)";
}

// 顶层语句也会被执行
var pi = Math.PI;
var msg = "hello.js loaded, PI = " + pi.toFixed(4);

// QJSEngine 安装了 ConsoleExtension 之后可用 console.log
if (typeof console !== "undefined") {
    console.log(msg);
}
