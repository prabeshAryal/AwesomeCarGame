# Create target folders if they don't exist
$folders = @("src", "include", "lib")
foreach ($folder in $folders) {
    if (-not (Test-Path $folder)) {
        New-Item -ItemType Directory -Path $folder | Out-Null
        Write-Host "Created folder: $folder"
    }
}

# Move .cpp files to src/
Get-ChildItem -Path . -Filter *.cpp | ForEach-Object {
    Move-Item $_.FullName -Destination "./src/" -Force
    Write-Host "Moved $($_.Name) → src/"
}

# Move .h and .hpp files to include/
Get-ChildItem -Path . -Include *.h, *.hpp -Recurse | ForEach-Object {
    Move-Item $_.FullName -Destination "./include/" -Force
    Write-Host "Moved $($_.Name) → include/"
}

# Move .dll files to lib/
Get-ChildItem -Path . -Filter *.dll | ForEach-Object {
    Move-Item $_.FullName -Destination "./lib/" -Force
    Write-Host "Moved $($_.Name) → lib/"
}

Write-Host "`n✅ Done organizing project files!"
