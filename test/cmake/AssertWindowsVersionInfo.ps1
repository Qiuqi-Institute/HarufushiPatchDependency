param(
    [Parameter(Mandatory = $true)]
    [string]$AppBinary
)

$version = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($AppBinary)

if ($version.FileDescription -ne "Harufushi Patch Dependency") {
    throw "Unexpected FileDescription: '$($version.FileDescription)'"
}
if ($version.CompanyName -ne "Aperip Daedalus Foundation") {
    throw "Unexpected CompanyName: '$($version.CompanyName)'"
}
if ($version.ProductName -ne "Harufushi Patch Dependency") {
    throw "Unexpected ProductName: '$($version.ProductName)'"
}
if ($version.LegalCopyright -ne "Copyright © 2026 Qiuqi Institute") {
    throw "Unexpected LegalCopyright: '$($version.LegalCopyright)'"
}
if ($version.Language -ne "Language Neutral") {
    throw "Expected language-neutral version resource, got '$($version.Language)'"
}
