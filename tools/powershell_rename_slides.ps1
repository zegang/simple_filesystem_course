# 设置编码为 UTF-8 解决中文乱码问题
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
[Console]::InputEncoding = [System.Text.Encoding]::UTF8

# 获取脚本所在目录下的所有文件
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Definition
$files = Get-ChildItem -Path $scriptPath -File

# 遍历每个文件
foreach ($file in $files) {
    # 检查文件名是否包含"幻灯片"（处理可能的编码问题）
    if ($file.Name -like "*幻灯片*") {
        $newName = $file.Name -replace "幻灯片", "slide"
        $newPath = Join-Path $file.Directory.FullName $newName

        if (Test-Path $newPath) {
            Write-Host "警告: 文件 $newName 已存在，跳过 $($file.Name)" -ForegroundColor Yellow
        } else {
            Rename-Item -Path $file.FullName -NewName $newName
            Write-Host "已重命名: $($file.Name) -> $newName" -ForegroundColor Green
        }
    }
}

Write-Host "按任意键退出..." -ForegroundColor Cyan
$host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")