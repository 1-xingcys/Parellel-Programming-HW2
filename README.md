# Parallel Programming HW2 - SIFT Algorithm Implementation

## 概述

本專案實作了 SIFT (Scale-Invariant Feature Transform) 演算法的平行化版本，採用 MPI + OpenMP 的混合平行模型來加速影像特徵點檢測。

## 實作架構

### 混合平行模型 (MPI + OpenMP)

- **MPI**: 用於進程間的任務分配
- **OpenMP**: 用於進程內的共享記憶體平行化

### 主要檔案結構

```
hw2/
├── hw2.cpp          # 主程式入口
├── sift.cpp         # SIFT 演算法實作
├── sift.hpp         # SIFT 相關標頭檔
├── image.cpp        # 影像處理函數
├── image.hpp        # 影像類別定義
├── Makefile         # 編譯設定
├── testcases/       # 測試影像
├── results/         # 輸出結果
└── goldens/         # 標準答案
```

## 平行化策略

### 1. OpenMP 平行化

在每個 MPI process 中，影像處理操作使用 OpenMP 進行平行化：

- 雙層迴圈使用 `#pragma omp parallel for collapse(2)` 將二維迴圈展開成單一迭代空間
- 提醒編譯器可以使用 ILP 相關指令，對簡單迴圈使用 `#pragma omp simd` 啟用硬體層級的平行化
- 輸出處理使用多執行緒安全的輸出機制，避免資料競爭

### 2. MPI 任務分解

採用基於 Octave 結構的任務分解策略：

- Gaussian Pyramid 由於序列相依性，每個進程都計算完整的金字塔
- Octave 使用交錯分配 (interleaved assignment)，進程 i 處理 Octaves i, i+size, i+2\*size...
- 交錯分配比區塊分配能更均勻地分散高解析度 Octave 的計算負載

## 編譯與執行

### 編譯

```bash
make
```

### 執行

```bash
# 基本執行
mpirun -np <processes> ./hw2 <input.jpg> <output.jpg> <output.txt>

# 範例
mpirun -np 4 ./hw2 ./testcases/01.jpg ./results/01.jpg ./results/01.txt
```

### 環境變數

```bash
# 設定 OpenMP 執行緒數
export OMP_NUM_THREADS=6
```

## 效能分析

### 負載平衡

- **進程間**: 由於 Octave 計算成本差異，負載平衡有限
- **執行緒間**: OpenMP 在進程內達到近乎完美的負載平衡

### 擴展性限制

- **進程數**: 受限於 Octave 總數 (8 個)，超過 8 個進程會有閒置
- **執行緒數**: 像素層級的平行化可有效利用更多 CPU 核心

## 主要函數

### image.cpp

- `Image::Image()`: 影像載入與格式轉換
- `Image::save()`: 影像儲存
- `rgb_to_grayscale()`: RGB 轉灰階
- `gaussian_blur()`: 高斯模糊
- `Image::resize()`: 影像縮放

### sift.cpp

- `find_keypoints_and_descriptors()`: 主要 SIFT 演算法
- `build_gaussian_pyramid()`: 建構高斯金字塔
- `build_dog_pyramid()`: 建構 DoG 金字塔
- `find_keypoints()`: 關鍵點檢測
- `compute_descriptors()`: 描述子計算

### hw2.cpp

- `main()`: 程式入口點，處理 MPI 初始化和結果輸出

## 驗證

使用提供的 `validate` 工具驗證結果：

```bash
./validate ./results/xx.txt ./goldens/xx.txt
```

## 參考資料

詳細的效能分析和實作細節請參考 `report.pdf`。
