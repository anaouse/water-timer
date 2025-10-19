import os
import subprocess
import sys
from pathlib import Path

# 编译器与通用参数
CC = "cl"
CFLAGS_COMMON = ["/std:c++20", "/utf-8", "/EHsc"]
INCLUDE = [
    "/I./SDL3/include",
    "/I.",
    "/I./imgui"
]
LIBPATH = [
    "/LIBPATH:./SDL3/lib/x64",
    "/LIBPATH:."
]
LIBRARIES = ["user32.lib", "gdi32.lib", "SDL3.lib"]

# 编译链接参数
CFLAGS = ["/MDd", "/Od", "/Zi",] + INCLUDE
LFLAGS = ["/link", "/SUBSYSTEM:CONSOLE"] + LIBPATH + LIBRARIES

# 生成的exe文件
MAIN_OUT = "water_timer.exe"
VIS_OUT = "visualizer.exe"

# 源文件列表
MAIN_SRC = ["main.cpp"]
VIS_SRC = [
    "visualizer.cpp",
    "imgui/imgui.cpp",
    "imgui/imgui_draw.cpp",
    "imgui/imgui_tables.cpp",
    "imgui/imgui_widgets.cpp",
    "imgui/backends/imgui_impl_sdl3.cpp",
    "imgui/backends/imgui_impl_sdlrenderer3.cpp"
]

# cpp->obj
def compile_obj(src, suffix=".obj"):
    src_path = Path(src)
    obj = (src_path.parent / (src_path.stem + suffix)).as_posix()
    print(f"--- Compiling Debug Object: {src} ---")
    cmd = [CC] + CFLAGS_COMMON + CFLAGS + ["/c", src, f"/Fo:{obj}"]
    subprocess.run(cmd, check=True)
    return obj

#objs->exe
def link_exe(objs, out):
    print(f"--- Linking Debug Executable: {out} ---")
    cmd = [CC] + objs + [f"/Fe:{out}"] + LFLAGS
    subprocess.run(cmd, check=True)


def build_main():
    objs = [compile_obj(f) for f in MAIN_SRC]
    link_exe(objs, MAIN_OUT)


def build_visualizer():
    objs = [compile_obj(f) for f in VIS_SRC]
    link_exe(objs, VIS_OUT)


def all():
    build_main()
    build_visualizer()
    print("--- Running Debug Executable (Console Mode) ---")
    subprocess.run([f".\\{MAIN_OUT}"])

def run():
    print("--- run the exe ---")
    subprocess.run([f"./{MAIN_OUT}"])

def clean():
    print("--- Cleaning up all built files ---")
    for ext in ("*.obj", "*.exe", "*.pdb", "*.ilk"):
        for f in Path(".").rglob(ext):
            try:
                print(f"delete {f.name}")
                f.unlink()
            except Exception:
                pass


if __name__ == "__main__":
    target = sys.argv[1] if len(sys.argv) > 1 else "run"

    if target == "main":
        build_main()
    elif target == "visualizer":
        build_visualizer()
    elif target == "run":
        run()
    elif target == "all":
        all()
    elif target == "clean":
        clean()
    else:
        print("Usage: python build.py [main|visualizer|run|all|clean]")
        print("main: build main.cpp")
        print("visualizer: build visualizer.cpp")
        print("run: run the exe")
        print("all: build main and visualizer and run")
        print("clean: clean all pdb, obj, ilk and exe files in current dir")

