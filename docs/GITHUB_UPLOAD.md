# GitHub 上传说明

这个目录已经按源码仓库方式整理，可直接作为 GitHub 仓库根目录。

## 可以直接提交

- `.gitignore`
- `CMakeLists.txt`
- `README.md`
- `dependencies.lock`
- `partitions.csv`
- `sdkconfig.defaults`
- `main/`
- `components/`
- `storage_data/`
- `tools/`
- `docs/`

## 不应提交

- `build/`
- `managed_components/`
- `sdkconfig`
- `sdkconfig.old`
- 本机日志、缓存和临时文件

## 克隆后编译

项目固定为 ESP32-S3-WROOM-1-N16R8。

```bash
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

如果曾在同一目录按经典 ESP32 编译过：

```bash
idf.py fullclean
idf.py set-target esp32s3
idf.py reconfigure
idf.py build
```

必要时删除自动生成的 `sdkconfig` / `sdkconfig.old` / `build/` 后重试，不要删除 `sdkconfig.defaults`。

ESP-IDF Component Manager 会根据 `main/idf_component.yml` 和 `dependencies.lock`
恢复未提交的 LVGL / esp_lvgl_port 等托管组件。
