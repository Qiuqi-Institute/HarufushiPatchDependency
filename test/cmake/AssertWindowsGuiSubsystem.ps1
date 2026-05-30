param(
    [Parameter(Mandatory = $true)]
    [string]$AppBinary
)

$bytes = [System.IO.File]::ReadAllBytes($AppBinary)
if ($bytes.Length -lt 0x100) {
    throw "Executable is too small to contain a valid PE header: $AppBinary"
}

$peOffset = [System.BitConverter]::ToInt32($bytes, 0x3c)
$optionalHeaderOffset = $peOffset + 24
$subsystemOffset = $optionalHeaderOffset + 0x44
$subsystem = [System.BitConverter]::ToUInt16($bytes, $subsystemOffset)

if ($subsystem -ne 2) {
    throw "Expected Windows GUI subsystem (2), got $subsystem"
}
