#!/usr/bin/env python3
import os
import subprocess
import sys
import glob

# Vanction编译器路径
VANCTION_EXEC = os.path.join(os.getcwd(), "build", "vanction.exe")

# 测试文件目录
TEST_DIR = os.path.join(os.getcwd(), "examples", "test")

# 确保编译器存在
if not os.path.exists(VANCTION_EXEC):
    print(f"Error: Vanction compiler not found at {VANCTION_EXEC}")
    print("Please build the project first using CMake")
    sys.exit(1)

# 确保测试目录存在
if not os.path.exists(TEST_DIR):
    print(f"Error: Test directory not found at {TEST_DIR}")
    sys.exit(1)

# 要忽略的测试文件列表
ignore_files = ["import_test_a.vn"]

# 获取所有测试文件
test_files = [f for f in glob.glob(os.path.join(TEST_DIR, "*.vn")) 
              if os.path.basename(f) not in ignore_files]

if not test_files:
    print(f"Warning: No .vn files found in {TEST_DIR}")
    sys.exit(0)

print(f"Found {len(test_files)} test files in {TEST_DIR}")
print("=" * 60)

# 测试结果
pass_count = 0
fail_count = 0

# 从命令行参数获取编译模式，默认为-g
if len(sys.argv) > 1:
    mode = sys.argv[1]
else:
    mode = "-g"

if mode not in ["-i", "-g"]:
    print("错误：请输入 -i 或 -g")
    exit(1)
# 运行测试
for test_file in test_files:
    filename = os.path.basename(test_file)
    print(f"Testing: {filename}")
    
    try:
        result = subprocess.run(
            [VANCTION_EXEC, mode, test_file],
            capture_output=True,
            text=True,
            timeout=15
        )
        
        # 检查结果
        # Vanction编译器返回main函数的返回值作为退出码，所以非零退出码不一定是错误
        has_error = bool(result.stderr.strip())
        
        if not has_error:
            print(f"  ✓ PASS")
            print(f"  Output: {result.stdout.strip()}")
            print(f"  Exit code: {result.returncode}")
            pass_count += 1
        else:
            print(f"  ✗ FAIL")
            print(f"  Exit code: {result.returncode}")
            print(f"  Error: {result.stderr.strip()}")
            print(f"  Output: {result.stdout.strip()}")
            fail_count += 1
            
    except subprocess.TimeoutExpired:
        print(f"  ✗ FAIL - Timeout")
        fail_count += 1
    except Exception as e:
        print(f"  ✗ FAIL - Exception: {e}")
        fail_count += 1
    
    
    print()

# 清理生成的.exe文件
for test_file in test_files:
    exe_file = test_file.replace(".vn", ".exe")
    if os.path.exists(exe_file):
        try:
            os.remove(exe_file)
        except Exception as e:
            print(f"Warning: Failed to remove {exe_file}: {e}")

print("=" * 60)
print(f"Test Results:")
print(f"  Total: {pass_count + fail_count}")
print(f"  Pass: {pass_count}")
print(f"  Fail: {fail_count}")
print("=" * 60)

if fail_count == 0:
    print("All tests passed! ✓")
    sys.exit(0)
else:
    print(f"{fail_count} tests failed! ✗")
    sys.exit(1)
