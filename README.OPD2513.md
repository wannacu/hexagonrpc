# OPD2513 传感器后端

本仓库基于 `linux-msm/hexagonrpc`，保存 OnePlus Pad 3 Pro OPD2513 所需的用户态传感器适配。主系统项目通过 `hexagonrpc/` Git 子模块固定配套版本。上游基线为 `da7a3742ab8d5273a281b70ee11f16fb2cb6a690`，完整上游说明见 [README.md](README.md)。

## 有效修改

- `hexagonrpcd/listener.c`：可选 RPC 返回错误后继续监听，失败调用不发送无效输出，并清理上一轮返回缓冲区。
- `hexagonrpcd/interface/adsp_listener.*`：使用 `get_in_bufs2` 补取较大请求；请求分配上限为 4 MiB。
- `hexagonrpcd/rpcd_builder.c`：映射 ODM 传感器配置，以及 Linux 侧的 persist 传感器数据副本。
- `hexagonrpcd/apps_std.c`、`hexagonfs*`：补充注册表写入、删除和文件描述符接入。
- `hexagonrpcd/sensor_registry.h`：将写入限制在专用注册表副本中，拒绝越界路径及符号链接目标。

## 构建

在 Linux 中安装 C 编译器、Meson 1.1 或更新版本、Ninja、Python 3 和 Linux 开发头文件。macOS 可使用 AArch64 Linux 构建虚拟机。

```sh
meson setup build --buildtype=debugoptimized -Db_ndebug=false -Dhexagonrpcd_verbose=false
meson compile -C build
meson test -C build --print-errorlogs
```

这些命令编译库、后端程序和回归测试，输出程序为 `build/hexagonrpcd/hexagonrpcd`。测试覆盖 RPC 错误处理、大消息及注册表路径限制，不需要连接平板。安装文件布局可先通过暂存目录检查：

```sh
DESTDIR="$PWD/stage" meson install -C build
```

编译出的程序需要配套 FastRPC 内核支持、设备 DSP 文件、传感器配置和系统服务才能在平板上工作。主项目负责这些部署组件；设备固件和校准数据不保存在本仓库。

## 运行时数据

`hexagonrpcd` 的 `-R` 指定设备文件根目录。新增目录包括：

| 文件根目录下的路径 | DSP 所见路径 |
| --- | --- |
| `sensors/odm-config/` | `/odm/etc/sensor/config/` |
| `sensors/persist/` | `/persist/sensors/`、`/mnt/vendor/persist/sensors/` |

运行服务通过环境变量 `PAD3PRO_SENSOR_REGISTRY` 指定 Linux 侧注册表副本的绝对目录。副本需包含 `registry/` 子目录，以及设备需要的注册表元数据。未设置该变量时拒绝注册表写入。不要将其指向 Android 的原始 persist 分区，也不要在公开仓库中提交设备校准副本。

## 许可

本组件沿用上游 GPL-3.0-or-later，完整文本见 [COPYING](COPYING)。各文件中的原有版权声明保留。它与内核、U-Boot 作为独立组件维护。
