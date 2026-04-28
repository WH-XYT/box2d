QT       += core qml
QT       -= gui

CONFIG   += c++17 console
CONFIG   -= app_bundle

TARGET   = qt_jsengine_example
TEMPLATE = app

SOURCES += main.cpp

# 把脚本拷到构建输出目录
scripts.path = $$OUT_PWD/scripts
scripts.files = $$PWD/scripts/*
INSTALLS += scripts

# 也在每次构建后拷贝一次（方便 IDE 直接 Run）
copy_scripts.commands = $(MKDIR) $$shell_path($$OUT_PWD/scripts) && \
                        $(COPY_DIR) $$shell_path($$PWD/scripts) $$shell_path($$OUT_PWD)
QMAKE_EXTRA_TARGETS += copy_scripts
POST_TARGETDEPS     += copy_scripts
