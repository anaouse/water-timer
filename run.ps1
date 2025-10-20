# start.ps1 内容
# 确保你将 YourProgram.exe 替换为你的实际程序名
$ExePath = ".\water_timer.exe"

# 创建一个 Shell 应用程序对象
$wsh = New-Object -ComObject WScript.Shell

# 运行程序
# 第一个参数是程序路径
# 第二个参数 '0' 是窗口样式参数，0 代表隐藏窗口 (Hide the window)
# 第三个参数 '$false' 表示不等待程序运行完成 (异步运行)
$wsh.Run($ExePath, 0, $false)
