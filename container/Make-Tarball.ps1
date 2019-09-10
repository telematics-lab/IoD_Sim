param (
    [string]$bsdtar          = 'tar',
    [string]$destinationPath = '.',
    [switch]$forceBsdtar     = $false
)

function Check-Command {
    param (
        [Parameter(Mandatory=$true)]
        [string]$command
    )

    Write-Host -NoNewline "==>  Is $command available? "
    Get-Command -ErrorAction SilentlyContinue $command
    if (isTrueOrFalse) {
        Write-Host 'YES'
        return $true
    } else {
        Write-Host 'NO'
        return $false
    }
}

function isTrueOrFalse {
    if ($?) { 
        return $true 
    } else { 
        return $false 
    }
}

Check-Command $bsdtar | Out-Null
$bsdtarAvailable = isTrueOrFalse

if (!$bsdtarAvailable) {
    Write-Error -ForegroundColor Red 'bsdtar not found!'
    return
}

'==>  Compressing...'
Remove-Item .\SNAPSHOT.tar -ErrorAction SilentlyContinue
Set-Location ../
Invoke-Expression "$bsdtar cf ../SNAPSHOT.tar ./*"
Move-Item -Force ../SNAPSHOT.tar ./container/
Set-Location container/
