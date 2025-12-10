
# NQueen 可视化程序

本项目实现了 N 皇后问题的求解及其可视化展示，并提供将结果导出为图像的功能。
界面基于 Qt 6（qtbase），使用 CMake 构建，建议在 Windows x64 (MSVC) 环境下使用 vcpkg 进行依赖管理。

---

## 项目结构说明

```
.
├─bin/                # 可执行文件与 Qt 运行库目录（build/Release 复制后）
│  ├─imageformats
│  ├─platforms
│  ├─styles
│  └─translations
├─build/              # 构建目录（用户本地生成）
│  └─...              # CMake 构建产物
├─img/                # 输出图片保存目录
├─requirements/       # 运行期依赖（vc_redist.x64）
└─src/                # 源码
   ├─common           # 通用工具或类型
   ├─core             # 求解算法与核心逻辑
   └─ui               # Qt 界面实现
```

> 注：`bin` 下的 Qt 平台插件目录由 Qt 自动生成或复制，运行时需完整保留。

---

## 构建说明

本项目采用 CMake，依赖 Qt 6（qtbase）。
推荐使用 vcpkg（Manifest Mode）自动安装依赖。

### vcpkg.json 示例

```json
{
  "name": "nqueen-viz",
  "version-string": "0.1.0",
  "builtin-baseline": "latest",
  "dependencies": [
    {
      "name": "qtbase",
      "platform": "windows & x64"
    }
  ]
}
```

---

### 构建流程

```bash
git clone https://github.com/dotm5/Nqueen.git
cd Nqueen

mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

说明：首次构建时 vcpkg 会根据清单自动安装 Qt（qtbase）。该步骤可能需要较长时间，属于正常现象。

---

## 运行前准备

请先执行以下安装包，以满足运行期依赖：

```
requirements/vc_redist.x64.exe
```

---

## 运行方法

构建完成后，可执行文件位于 `bin` 目录：

```
bin/NQueensViz.exe
```

也可以通过文件资源管理器直接运行。

---

## 输出结果

每次求解后生成的棋盘图像将保存至：

```
img/
```

如需清空结果，可直接删除该目录下内容。

---

## 扩展与开发说明

源代码主要分为三个部分：

* `src/core`：求解算法及相关逻辑（可扩展算法策略，例如回溯优化、启发式策略等）
* `src/ui`：Qt 图形界面逻辑
* `src/common`：通用工具（配置、类型、辅助函数等）

若需要进一步扩展，可从 `core` 目录入手修改算法实现，或者在 `ui` 中增加显示方式。

---

## 编译环境建议

* Windows 10 或更高版本
* Visual Studio 2022 / MSVC
* CMake 3.20 及以上
* vcpkg（Manifest Mode）

