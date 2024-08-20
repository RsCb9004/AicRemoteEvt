# AIC Remote Event

**注：此插件仅可运行于 Windows 系统。**

这是 ACT 游戏 Alice In Cradle 的插件。此插件允许您在另一个可执行文件中远程操控 AIC 游戏程序执行 HashCode 指令。

通过此插件，游戏中指令的执行变得更便捷。此插件可以辅助事件的编写调试等。

## 原理

`are-client.exe` 调用 Win32 API 向窗口发送消息。`AicRemoteEvt.dll` 调用 Win32 API 设置 Hook 监听这些消息，并在游戏中执行。

## 使用

1. 在 AIC 游戏目录中安装 [BepInEx](https://github.com/BepInEx/BepInEx)。
2. 在 [Release 页面](https://github.com/RsCb9004/AicRemoteEvt/releases) 中下载 `AicRemoteEvt-win-x.x.zip` 并解压。
3. 将解压后的 `AicRemoteEvt.dll` 放入游戏目录的 `\BepInEx\plugins\` 文件夹中。
4. 运行游戏并载入存档。
5. 运行解压后的 `are-client.exe`。一般情况下，当询问窗口名称时，直接按回车使用默认窗口名称（AliceInCradle）即可。之后，一般来说，如果输出的窗口句柄不为 `0x00000000`，则找到了正确的窗口。
6. 输入哈语言命令并按回车即可在游戏游戏中执行该命令。

## `are-client.exe` 的详细使用方法

`are-client.exe` 可以给指定窗口发送消息。

**注意：发送的消息大小无论如何也不会超过 65535 byte，因为程序的缓冲区就这么大。如果要修改缓冲区大小，参见 [编译 `are-client.cpp`](#编译-are-clientcpp)。**

### 命令行参数

#### 基本用法
```
are-client.exe [ [ -H <HANDLE> | -N <WINDOW_NAME> | -C <CLASS_NAME> ] [-p <PARAM>] [-q] | -h | -v ]
```

#### 参数详解

- `-H`，`--handle`：指定目标窗口的句柄（64 位整数）。
- `-N`, `--name`：指定目标窗口的名称，程序将寻找第一个符合条件的窗口。
- `-C`, `--class`：指定目标窗口的类名称，程序将寻找第一个符合条件的窗口。

以上三个参数用于指定窗口。窗口最多只能指定一个。若未在启动参数中指定，会在程序运行时询问窗口名称。

- `-d`, `--data`：指定要发送的附加数据（64 位整数）。默认为 `0x6d756b6f75616f69`。`AicRemoteEvt.dll` 只接收为此值的信息，其他的将被忽略。
- `-q`，`--quiet`：在安静模式下执行。程序不会输出任何信息，并会一直将命令读到 EOF(`^Z`) 再一起发送。若未指定窗口，会直接使用默认窗口名称寻找窗口。可在输入重定向时使用。
- `-h`, `--help`：显示帮助信息。显示内容与本节内容相似。
- `-v`，`--version`：显示程序版本。

### 控制台模式

如果程序不是以安静模式启动，那么它就是控制台模式。

---

在控制台模式中，您可以在 `$` 后写命令，然后按回车键发送：
```
$FOO
```
实际发送的字符串：
```
FOO
```

（实际发送的字符串最后还有一个换行符，下同。不过这一换行符一般并不会有影响，所以将其忽略。）

---

如果括号不匹配，那么命令不会发送。您可以在新一行的 `.` 后继续您的命令，直到括号匹配：
```
$IF bar {
.    FOO
.}
```
实际发送的字符串：
```
IF bar {
    FOO
}
```

---

如果在括号匹配的情况下仍想输入多行命令，可以在一行末尾加 `.`（`.` 并不会算入消息中）：
```
$FOO .
.BAR
```
实际发送的字符串：
```
FOO
BAR
```

---

如果觉得行数很多，每行末尾都打 `.` 很累，可以输入 `...` 然后按回车，之后无论多少行都会一直读取。输入 `..` 结束命令。（`...`，`..` 并不会算入消息中）：
```
$...
FOO
BAR
..
```
实际发送的字符串：
```
FOO
BAR
```

## 编译 `are-client.cpp`

`are-client.cpp` 归根结底只是消息发送器，并非只能用于 AIC Remote Event。通过以下操作可以在编译时修改一些默认值，以将其适用于其它功能。

您可以在编译选项中修改以下宏：`WNAME_SIZE`，`MSG_SIZE`，`WNAME_DEFAULT`，`DWDATA_DEFAULT`。

- `WNAME_SIZE`：输入窗口名称的缓冲区大小。
- `MSG_SIZE`：输入消息的缓冲区大小。
- `WNAME_DEFAULT`：默认窗口名称。
- `DWDATA_DEFAULT`：默认附加信息。

以 g++ 为例，若要将 `WNAME_DEFAULT` 修改为 `"foo"`，添加编译选项 `-DWNAME_DEFAULT=\"foo\"`。

总的命令可以是：
```
g++ .\are-client.cpp -o .\foo-client-exe -Wall -O2 -DWNAME_DEFAULT=\"foo\"
```
